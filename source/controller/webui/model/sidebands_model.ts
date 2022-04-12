export const kNumGenerators = 8;

export enum ParamTag {
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
    TAG_MODULATIONS,
}

export enum TargetTag {
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

export interface Tag {
    Generator: number;
    Param: ParamTag;
    Target: TargetTag;
}

export function ParseTag(paramID: number): Tag {
    return {
        Generator: paramID >> 24,
        Target: ((paramID & 0x00ffff00) >> 8),
        Param: paramID & 0x000000ff,
    };
}

export function pid(g: number, p: ParamTag, t: TargetTag): number {
    return (g << 24 | t << 8 | p);
}

export function ParamIDFor(tag: Tag): number {
    return pid(tag.Generator, tag.Param, tag.Target);
}

export interface EnvelopeStageChangeMessage {
    messageID: string;
    gennum: number;
    noteId: number;
    target: number;
    envelopeStage: number;
}

export interface AnalysisBufferMessage {
    messageID: string;
    sampleRate: number;
    bufferSize: number;
    frequency: number;
    gennum: number;
    bufferData: Float64Array;
}

