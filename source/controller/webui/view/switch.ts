import * as Model from "../model/sidebands_model";
import {ParamIDFor, ParamTag, Tag, TargetTag} from "../model/sidebands_model";
import {GeneratorView} from "./views";
import {controller, IDependent, IParameter, Message} from "../model/vst_model";

export class Switch implements GeneratorView, IDependent {
    constructor(readonly element: HTMLImageElement,
                private tag : Tag) {
        element.addEventListener("click", (c) => {
            this.toggle();
        });
        controller.subscribeParameter(ParamIDFor(this.tag), this);
        this.refresh();
    }

    toggle() {
        const tag = ParamIDFor(this.tag);
        controller.getParameterObject(tag).then((p) => {
            console.log(this.tag);
            const newValue = p.normalized == 0 ? 1 : 0;
            controller.beginEdit(tag).then(v=>{
                controller.setParamNormalized(tag, newValue).then(v=>{
                    controller.performEdit(tag, newValue).then(v => {
                        controller.endEdit(tag);
                    });

                });
            })
        })
    }

    refresh() {
        controller.getParameterObject(ParamIDFor(this.tag)).then((p) => {
            this.changed(p);
        })
    }

    changed(parameter: IParameter): void {
        if (parameter.normalized == 0) {
            this.element.src = "MetalSwitchOff.png"
        } else {
            this.element.src = "MetalSwitchOn.png"
        }
    }

    updateSelectedGenerator(gennum: number): void {
        this.tag.Generator = gennum;
        controller.subscribeParameter(ParamIDFor(this.tag), this);
        this.refresh();
    }

    node(): HTMLElement {
        return this.element;
    }
}