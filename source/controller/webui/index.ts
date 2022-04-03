import * as Model from './sidebandsModel';
import {IParameter, IRangeParameter} from "./sidebandsModel";
import {addKnob, GeneratorTab, addTab} from "./controls";

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
