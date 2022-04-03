import {addKnob, IParameterControl} from "./controls";
import * as Model from "../model/sidebands_model";
import {MakeEnvelopeEditor} from "./templates";
import {GD, GeneratorView} from "./views";

export class EnvelopeEditorKnobView implements GeneratorView {
    controls: Array<IParameterControl>;

    constructor(readonly element :HTMLDivElement, readonly gennum : number, target : Model.TargetTag) {
        this.controls = [];
        const targetPrefix = target.toString().toLowerCase();
        let panel = MakeEnvelopeEditor(targetPrefix);
        element.appendChild(panel);

        this.makeEnvelopeKnobs(targetPrefix, gennum, target);
    }

    updateSelectedGenerator(gennum: number): void {
        for (let control of this.controls) {
            control.updateSelectedGenerator(gennum);
        }
    }

    makeEnvelopeKnobs(elemPrefix: string, gennum: number, target: Model.TargetTag) {
        let pushControl = (c: IParameterControl) => {
            this.controls.push(c);
        };

        addKnob(GD(`${elemPrefix}_hold_time`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_HT, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_attack`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_AR, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_d1_r`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_DR1, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_d2_r`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_DR2, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_r1_r`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_RR1, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_r2_r`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_RR2, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_attack_level`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_AL, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_d1_l`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_DL1, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_s_l`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_SL, Target: target}).then(pushControl);
        addKnob(GD(`${elemPrefix}_r1_l`),
            {Generator: gennum, Param: Model.ParamTag.TAG_ENV_RL1, Target: target}).then(pushControl);
    }

    node() {
        return this.element;
    }
}
