import {createKnob} from './pureknob';
import {MakeTab} from './templates';
import * as Model from './sidebandsModel';
import {IParameter, IRangeParameter} from "./sidebandsModel";

// Exported EditController functions.
export declare function beginEdit(tag: number): Promise<void>;
export declare function performEdit(tag: number, value: number): Promise<void>;
export declare function endEdit(tag: number): Promise<void>;
export declare function setParamNormalized(tag: number, value: number): Promise<void>;
export declare function getParameterObject(tag: number): Promise<IParameter>;
export declare function getParameterObjects(tag: Array<number>): Promise<{ [key: number]: IParameter }>;
export declare function getSelectedUnit(): Promise<number>;
export declare function selectUnit(unitId: number) : Promise<void>;

// Shortcut.
function GD(id: string): HTMLElement | null {
    const el = document.getElementById(id);
    if (!el) {
        console.log(`Unable to find: ${id}`);
    }
    return el;
}

class BaseParameterControl<ElementType extends HTMLElement> {
    constructor(readonly pTag: Model.Tag, readonly element: ElementType) {
        this.pTag = pTag;
        this.element = element;
    }

    node(): HTMLElement {
        return this.element;
    }

    setValue(val: number) {
        let pid = Model.ParamIDFor(this.pTag);
        beginEdit(pid).then(x => {
            setParamNormalized(pid, val).then(x => {
            });
            performEdit(pid, val).then(x => {
                endEdit(pid).then(x => {
                })
            })
        })
    }

}

class Toggle extends BaseParameterControl<HTMLInputElement> {
    constructor(pTag: Model.Tag, toggle: HTMLInputElement) {
        toggle.addEventListener('change', () => {
            this.setValue(this.element.checked ? 1 : 0);
        })
        super(pTag, toggle);
    }
}

class ParameterKnob extends BaseParameterControl<HTMLElement> {
    readonly knobControl: any;
    readonly parameter: Model.IRangeParameter;

    constructor(tag: Model.Tag, parameter: Model.IRangeParameter) {
        let knobControl = createKnob(48, 48);
        let properties = knobControl.properties;
        properties.angleStart = -0.75 * Math.PI;
        properties.angleEnd = 0.75 * Math.PI;
        properties.colorFG = '#fb4400';
        properties.trackWidth = 0.4;
        properties.valMin = parameter.min;
        properties.valMax = parameter.max;

        super(tag, knobControl.node());

        this.knobControl = knobControl;
        this.parameter = parameter;

        knobControl.value = this.normalizedToPlain(parameter.normalized);
        knobControl.addListener(this.knobListener.bind(this));
    }

    normalizedToPlain(normalized: number): number {
        return this.parameter.min + normalized * (this.parameter.max - this.parameter.min);
    }

    plainToNormalized(plain: number): number {
        return (plain - this.parameter.min) / this.parameter.max;
    }

    knobListener(knobControl: any, value: number) {
        const val = this.plainToNormalized(value);
        this.setValue(val);
    }

    node(): HTMLElement {
        return this.knobControl.node();
    }
}

class GeneratorTab {
    readonly element: HTMLElement;
    readonly toggle: Toggle;

    constructor(readonly gennum: number, parent: HTMLElement) {
        this.gennum = gennum;

        const rootTabElement = MakeTab(gennum);
        const toggleInput = rootTabElement.querySelector(`#generator_${gennum}_toggle`);
        this.toggle = new Toggle({
            Generator: gennum,
            Param: Model.ParamTag.TAG_GENERATOR_TOGGLE,
            Target: Model.TargetTag.TARGET_NA
        }, <HTMLInputElement>toggleInput);

        const knobElement = rootTabElement.querySelector(`#generator_${gennum}_level`);
        addKnob(knobElement, {
            Generator: gennum,
            Param: Model.ParamTag.TAG_OSC,
            Target: Model.TargetTag.TARGET_A
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

function buildKnob(elem: Element, tag: Model.Tag, param: Model.IRangeParameter) {
    const knob = new ParameterKnob(tag, param);
    const node = knob.node();
    elem.appendChild(node);
    return knob;
}

function addKnob(elem: Element | null, tag: Model.Tag) {
    return new Promise<ParameterKnob>((resolve) => {
        getParameterObject(Model.ParamIDFor(tag)).then((param) => {
            if (param.isRangeParameter) {
                resolve(buildKnob(<HTMLElement>elem, tag, <IRangeParameter>param));
            }
        })
    });
}

function addTab(parent: HTMLElement | null, gennum: number): GeneratorTab | null {
    if (!parent) return null;

    let tab = new GeneratorTab(gennum, parent);

    return tab;
}

function makeEnvelopeKnobs(elemPrefix: string, target: Model.TargetTag) {
    addKnob(GD(`${elemPrefix}_hold_time`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_HT, Target: target});
    addKnob(GD(`${elemPrefix}_attack`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_AR, Target: target});
    addKnob(GD(`${elemPrefix}_d1_r`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_DR1, Target: target});
    addKnob(GD(`${elemPrefix}_d2_r`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_DR2, Target: target});
    addKnob(GD(`${elemPrefix}_r1_r`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_RR1, Target: target});
    addKnob(GD(`${elemPrefix}_r2_r`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_RR2, Target: target});
    addKnob(GD(`${elemPrefix}_attack_level`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_AL, Target: target});
    addKnob(GD(`${elemPrefix}_d1_l`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_DL1, Target: target});
    addKnob(GD(`${elemPrefix}_s_l`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_SL, Target: target});
    addKnob(GD(`${elemPrefix}_r1_l`),
        {Generator: 0, Param: Model.ParamTag.TAG_ENV_RL1, Target: target});
}

export function ready() {
    console.log("Awoken.");

    const selector_area = GD('generator_selector');
    let generators: Array<GeneratorTab> = [];
    for (let x = 0; x < 12; x++) {
        let tab = addTab(selector_area, x);
        if (tab)
            generators.push(tab);
    }
    generators[0].select();

    addKnob(GD('carrier_ratio'),
        {Generator: 0, Param: Model.ParamTag.TAG_OSC, Target: Model.TargetTag.TARGET_C});

    addKnob(GD('modulation_ratio'),
        {Generator: 0, Param: Model.ParamTag.TAG_OSC, Target: Model.TargetTag.TARGET_M});
    addKnob(GD('modulation_index'),
        {Generator: 0, Param: Model.ParamTag.TAG_OSC, Target: Model.TargetTag.TARGET_K});

    makeEnvelopeKnobs('a', Model.TargetTag.TARGET_A);
    makeEnvelopeKnobs('k', Model.TargetTag.TARGET_K);

    console.log("Done");
}

document.addEventListener('DOMContentLoaded', ready, false);
