import * as Model from './sidebandsModel';
import {IParameter, IRangeParameter} from "./sidebandsModel";
import {createKnob} from "./pureknob";
import {MakeTab} from "./templates";

// Exported EditController functions.
export declare function beginEdit(tag: number): Promise<void>;
export declare function performEdit(tag: number, value: number): Promise<void>;
export declare function endEdit(tag: number): Promise<void>;
export declare function setParamNormalized(tag: number, value: number): Promise<void>;
export declare function getParameterObject(tag: number): Promise<IParameter>;
export declare function getParameterObjects(tag: Array<number>): Promise<{ [key: number]: IParameter }>;
export declare function getSelectedUnit(): Promise<number>;
export declare function selectUnit(unitId: number) : Promise<void>;

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

export class GeneratorTab {
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

export function addKnob(elem: Element | null, tag: Model.Tag) {
    return new Promise<ParameterKnob>((resolve) => {
        getParameterObject(Model.ParamIDFor(tag)).then((param) => {
            if (param.isRangeParameter) {
                resolve(buildKnob(<HTMLElement>elem, tag, <IRangeParameter>param));
            }
        })
    });
}

export function addTab(parent: HTMLElement | null, gennum: number): GeneratorTab | null {
    if (!parent) return null;

    let tab = new GeneratorTab(gennum, parent);

    return tab;
}
