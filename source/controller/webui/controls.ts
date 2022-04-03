import * as SidebandsModel from './sidebandsModel';
import * as VstModel from "./vstModel";
import {createKnob} from "./pureknob";
import {MakeTab} from "./templates";

// Exported EditController functions.

export interface IParameterControl {
    readonly pTag: SidebandsModel.Tag;
}

export class ParameterControl implements IParameterControl {
    constructor(readonly pTag : SidebandsModel.Tag) {}

    setValue(val: number) {
        let pid = SidebandsModel.ParamIDFor(this.pTag);
        VstModel.controller.beginEdit(pid).then(x => {
            VstModel.controller.setParamNormalized(pid, val).then(x => {
            });
            VstModel.controller.performEdit(pid, val).then(x => {
                VstModel.controller.endEdit(pid).then(x => {
                })
            })
        })
    }
}

class BaseParameterControlView<ElementType extends HTMLElement> extends ParameterControl {
    constructor(readonly pTag: SidebandsModel.Tag, readonly element: ElementType) {
        super(pTag);
        this.element = element;
    }

    node(): HTMLElement {
        return this.element;
    }
}

export class Toggle extends BaseParameterControlView<HTMLInputElement> {
    constructor(pTag: SidebandsModel.Tag, toggle: HTMLInputElement) {
        toggle.addEventListener('change', () => {
            this.setValue(this.element.checked ? 1 : 0);
        })
        super(pTag, toggle);
    }
}

export class ParameterKnob extends BaseParameterControlView<HTMLElement> {
    readonly knobView: any;
    readonly parameter: VstModel.IRangeParameter;

    constructor(tag: SidebandsModel.Tag, parameter: VstModel.IRangeParameter) {
        let knobView = createKnob(48, 48);
        let properties = knobView.properties;
        properties.angleStart = -0.75 * Math.PI;
        properties.angleEnd = 0.75 * Math.PI;
        properties.colorFG = '#fb4400';
        properties.trackWidth = 0.4;
        properties.valMin = parameter.min;
        properties.valMax = parameter.max;

        super(tag, knobView.node());

        this.knobView = knobView;
        this.parameter = parameter;

        knobView.value = this.normalizedToPlain(parameter.normalized);
        knobView.addListener(this.knobListener.bind(this));
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
        return this.knobView.node();
    }
}

function buildKnob(elem: Element, tag: SidebandsModel.Tag, param: VstModel.IRangeParameter) {
    const knob = new ParameterKnob(tag, param);
    const node = knob.node();
    elem.appendChild(node);
    return knob;
}

export function addKnob(elem: Element | null, tag: SidebandsModel.Tag) : Promise<ParameterKnob> {
    return new Promise<ParameterKnob>((resolve) => {
        VstModel.controller.getParameterObject(SidebandsModel.ParamIDFor(tag)).then((param) => {
            if (param.isRangeParameter) {
                resolve(buildKnob(<HTMLElement>elem, tag, <VstModel.IRangeParameter>param));
            }
        })
    });
}

