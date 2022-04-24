import {GeneratorView} from "./views";
import {IDependent, IMsgSubscriber, IParameter} from "../model/vst_model";
import {ParamIDFor, ParamTag, TargetTag} from "../model/sidebands_model";
import * as VstModel from "../model/vst_model";

const kEnvelopeModulator = 1;
const kLFOModulator = 2;
export class ModRowView implements GeneratorView, IDependent {
    modulatorType : number;
    gennum : number;
    target : TargetTag;
    subViews: Array<GeneratorView>;

    constructor(readonly element : HTMLDivElement, gennum : number, target : TargetTag, modulatorType : number) {
        this.gennum = gennum;
        this.target = target;
        this.modulatorType = modulatorType;

        this.subViews = [];
    }

    redraw() : void {

    }

    changed(parameter: IParameter): void {

    }

    node(): HTMLElement {
        return this.element;
    }

    updateSelectedGenerator(gennum: number): void {
    }


}