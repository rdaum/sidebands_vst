import {createKnob} from './pureknob';
import {MakeTab} from './templates';

function GD(id:string) : HTMLElement | null{
    const el = document.getElementById(id);
    if (!el) {
        console.log(`Unable to find: ${id}`);
    }
    return el;
}

enum ParamTag {
    TAG_GENERATOR_SELECT,
    TAG_GENERATOR_TOGGLE,
    TAG_OSC,
    TAG_ENV_HT,
    TAG_ENV_AR,
    TAG_ENV_AL,
    TAG_ENV_DR1,
    TAG_ENV_DL1,
    TAG_ENV_DR2,
    TAG_ENV_SL, // sustain
    TAG_ENV_RR1,
    TAG_ENV_RL1,
    TAG_ENV_RR2,
    TAG_ENV_VS,
    TAG_LFO_FREQ,
    TAG_LFO_AMP,
    TAG_LFO_VS,
    TAG_LFO_TYPE,
    TAG_SELECTED_GENERATOR, // valid only with "0" for generator and TARGET_NA
    TAG_MOD_TYPE,
}

enum TargetTag {
    TARGET_NA,
    TARGET_C,
    TARGET_A,
    TARGET_M,
    TARGET_K,
    TARGET_R,
    TARGET_S,
    TARGET_PORTAMENTO,
    TARGET_OSC_TYPE,
}

interface Tag {
    Generator: number;
    Param: ParamTag;
    Target: TargetTag;
}

function ParseTag(paramID: number): Tag {
    let parsed = {
        Generator: paramID >> 24,
        Target: ((paramID & 0x00ffff00) >> 8),
        Param: paramID & 0x000000ff,
    };
    return parsed;
}

function pid(g: number, p: ParamTag, t: TargetTag): number {
    return (g << 24 | t << 8 | p);
}

function ParamIDFor(tag: Tag): number {
    return pid(tag.Generator, tag.Param, tag.Target);
}

interface IParameterInfo {
    id: number;
    title: string;
    shortTitle: string;
    stepCount: number;
    flags: number;
    defaultNormalizedValue: number;
    units: number;
}

interface IParameter {
    normalized: number;
    precision: number;
    unitID: number;
    info: IParameterInfo;
}

class ParameterHandle {
    id: number;
    param: IParameter;

    constructor(id: number, param: IParameter) {
        this.id = id;
        this.param = param;
    }
}

// Exported EditController functions.
declare function beginEdit(tag: number): Promise<void>;

declare function performEdit(tag: number, value: number): Promise<void>;

declare function endEdit(tag: number): Promise<void>;

declare function setParamNormalized(tag: number, value: number): Promise<void>;

declare function getParameterObject(tag: number): Promise<IParameter>;

declare function getParameterObjects(tag: Array<number>): Promise<{ [key: number]: IParameter }>;

class BaseParameterControl<ElementType extends HTMLElement> {
    pTag: Tag;
    element: ElementType;

    constructor(pTag: Tag, element :ElementType) {
        this.pTag = pTag;
        this.element = element;
    }

    node(): HTMLElement {
        return this.element;
    }

    setValue(val: number) {
        let pid = ParamIDFor(this.pTag);
        beginEdit(pid).then(x => {
            setParamNormalized(pid, val).then(x => {
                console.log("Set: ", this.pTag.Param, " ", this.pTag.Target);

            });
            performEdit(pid, val).then(x => {
                endEdit(pid).then(x => {
                    console.log("Complete edit.");
                })
            })
        })
    }

}

class Toggle extends BaseParameterControl<HTMLInputElement> {
    constructor(pTag: Tag, toggle : HTMLInputElement) {
        toggle.addEventListener('change', () => {
            this.setValue(this.element.checked ? 1 : 0);
        })
        super(pTag, toggle);
    }
}

class ParameterKnob extends BaseParameterControl<HTMLElement> {
    knobControl: any;
    parameter: IParameter;
    min: number;
    max: number;

    constructor(tag: Tag, parameter: IParameter, min: number, max: number) {
        let knobControl = createKnob(48, 48);
        let properties = knobControl.getProperties();
        properties.angleStart =  -0.75 * Math.PI;
        properties.angleEnd = 0.75 * Math.PI;
        properties.colorFG = '#fb4400';
        properties.trackWidth = 0.4;
        properties.valMin = min;
        properties.valMax = max;

        super(tag, knobControl.node());

        this.knobControl = knobControl;
        this.parameter = parameter;
        this.min = min;
        this.max = max;


        knobControl.setValue(this.normalizedToPlain(parameter.normalized));
        knobControl.addListener(this.knobListener.bind(this));
    }

    normalizedToPlain(normalized: number): number {
        return this.min + normalized * (this.max - this.min);
    }

    plainToNormalized(plain: number): number {
        return (plain - this.min) / this.max;
    }

    knobListener(knobControl: any, value: number) {
        const val = this.plainToNormalized(value);
        this.setValue(value);
    }

    node(): HTMLElement {
        return this.knobControl.node();
    }
}

class GeneratorTab {
    gennum: number;
    element: HTMLElement;
    toggle: Toggle;

    constructor(gennum: number, parent : HTMLElement) {
        this.gennum = gennum;

        const rootTabElement = MakeTab(gennum);
        const toggleInput = rootTabElement.querySelector(`#generator_${gennum}_toggle`);
        this.toggle = new Toggle({
            Generator: gennum,
            Param: ParamTag.TAG_GENERATOR_TOGGLE,
            Target: TargetTag.TARGET_NA
        }, <HTMLInputElement>toggleInput);

        const knobElement = rootTabElement.querySelector(`#generator_${gennum}_level`);
        addKnob(knobElement, {
            Generator: gennum,
            Param: ParamTag.TAG_OSC,
            Target: TargetTag.TARGET_A
        })

        rootTabElement.addEventListener("click", () => {
            this.select();
        })
        this.element = rootTabElement;
        parent.appendChild(this.element);
    }

    select() {
        let element = this.element;

        const selected = element.getAttribute('is-selected');
        if (selected != "false") return;

        element.setAttribute('is-selected', 'true');
        element.classList.replace('tab-inactive', 'tab-active');

        let parent = element.parentNode;
        if (!parent) return;
        for (let tabSpan of parent.children) {
            if (tabSpan == element) {
                continue;
            }
            const childSelected = tabSpan.getAttribute('is-selected');
            if (childSelected == "false") {
                continue;
            }
            tabSpan.setAttribute('is-selected', "false");
            tabSpan.classList.replace('tab-active', 'tab-inactive');
        }
    }

    node(): HTMLElement {
        return this.element;
    }
}

function buildKnob(elem: Element, tag: Tag, param: IParameter) {
    const knob = new ParameterKnob(tag, param, 0, 10);
    const node = knob.node();
    elem.appendChild(node);
    return knob;
}

function addKnob(elem: Element | null, tag: Tag) {
    return new Promise<ParameterKnob>( (resolve) => {
        getParameterObject(ParamIDFor(tag)).then((param) => {
            resolve(buildKnob(<HTMLElement>elem, tag, param));
        })
    });
}

function addTab(parent: HTMLElement | null, gennum: number) : GeneratorTab | null {
    if (!parent) return null;

    let tab = new GeneratorTab(gennum, parent);

    return tab;
}

function makeEnvelopeKnobs(elemPrefix: string, target: TargetTag) {
    addKnob(GD(`${elemPrefix}_hold_time`),
        {Generator: 0, Param: ParamTag.TAG_ENV_HT, Target: target});
    addKnob(GD(`${elemPrefix}_attack`),
        {Generator: 0, Param: ParamTag.TAG_ENV_AR, Target: target});
    addKnob(GD(`${elemPrefix}_d1_r`),
        {Generator: 0, Param: ParamTag.TAG_ENV_DR1, Target: target});
    addKnob(GD(`${elemPrefix}_d2_r`),
        {Generator: 0, Param: ParamTag.TAG_ENV_DR2, Target: target});
    addKnob(GD(`${elemPrefix}_r1_r`),
        {Generator: 0, Param: ParamTag.TAG_ENV_RR1, Target: target});
    addKnob(GD(`${elemPrefix}_r2_r`),
        {Generator: 0, Param: ParamTag.TAG_ENV_RR2, Target: target});
    addKnob(GD(`${elemPrefix}_attack_level`),
        {Generator: 0, Param: ParamTag.TAG_ENV_AL, Target: target});
    addKnob(GD(`${elemPrefix}_d1_l`),
        {Generator: 0, Param: ParamTag.TAG_ENV_DL1, Target: target});
    addKnob(GD(`${elemPrefix}_s_l`),
        {Generator: 0, Param: ParamTag.TAG_ENV_SL, Target: target});
    addKnob(GD(`${elemPrefix}_r1_l`),
        {Generator: 0, Param: ParamTag.TAG_ENV_RL1, Target: target});
}

export function ready() {
    console.log("Awoken.");

    const selector_area = GD('generator_selector');
    let generators : Array<GeneratorTab> = [];
    for (let x = 0; x < 12; x++) {
        let tab = addTab(selector_area, x);
        if (tab)
            generators.push(tab);
    }
    generators[0].select();

    addKnob(GD('carrier_ratio'),
        {Generator: 0, Param: ParamTag.TAG_OSC, Target: TargetTag.TARGET_C});

    addKnob(GD('modulation_ratio'),
        {Generator: 0, Param: ParamTag.TAG_OSC, Target: TargetTag.TARGET_M});
    addKnob(GD('modulation_index'),
        {Generator: 0, Param: ParamTag.TAG_OSC, Target: TargetTag.TARGET_K});

    makeEnvelopeKnobs('a', TargetTag.TARGET_A);
    makeEnvelopeKnobs('k', TargetTag.TARGET_K);

    console.log("Done");
}

document.addEventListener('DOMContentLoaded', ready, false);
