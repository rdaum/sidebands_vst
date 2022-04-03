import * as Model from './sidebandsModel';
import {IParameter, IRangeParameter, ParamTag} from "./sidebandsModel";
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

export declare function selectUnit(unitId: number): Promise<void>;

export interface ParameterControl {
    readonly pTag: Model.Tag;
}

class BaseParameterControl<ElementType extends HTMLElement> implements ParameterControl {
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

export class Toggle extends BaseParameterControl<HTMLInputElement> {
    constructor(pTag: Model.Tag, toggle: HTMLInputElement) {
        toggle.addEventListener('change', () => {
            this.setValue(this.element.checked ? 1 : 0);
        })
        super(pTag, toggle);
    }
}

export class ParameterKnob extends BaseParameterControl<HTMLElement> {
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

