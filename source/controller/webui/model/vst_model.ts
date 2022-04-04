
export interface IParameterInfo {
    id: number;
    title: string;
    shortTitle: string;
    stepCount: number;
    flags: number;
    defaultNormalizedValue: number;
    units: number;
}

export interface IParameter {
    normalized: number;
    precision: number;
    unitID: number;
    info: IParameterInfo;
    isRangeParameter: boolean;
}

export interface IRangeParameter extends IParameter {
    min: number;
    max: number;
}

export function ValueOf(rp : IRangeParameter | null) : number {
    if (!rp) return 0;
    return rp.min + rp.normalized * (rp.max - rp.min);
}



export declare function beginEdit(tag: number): Promise<void>;

export declare function performEdit(tag: number, value: number): Promise<void>;

export declare function endEdit(tag: number): Promise<void>;

export declare function setParamNormalized(tag: number, value: number): Promise<void>;

export declare function getParameterObject(tag: number): Promise<IParameter>;

export declare function getParameterObjects(tag: Array<number>): Promise<{ [key: number]: IParameter }>;

export declare function getSelectedUnit(): Promise<number>;

export declare function selectUnit(unitId: number): Promise<void>;

export declare function subscribeParameter(tag: number) : Promise<void>;

export interface IDependent {
    changed(parameter : IParameter) : void;
}

type Dependencies = {[tag: number]: Array<IDependent>};
export class Controller {
    private dependencies : Dependencies;

    constructor() {
        this.dependencies = {};
    }

    subscribeParameter(tag : number, dependent : IDependent) {
        if (!this.dependencies[tag])
            this.dependencies[tag] = [];
        this.dependencies[tag].push(dependent);
    }

    notifyParameterChange(parameter :IParameter) : void {
        const dependencies = this.dependencies[parameter.info.id];
        if (dependencies) {
            for (const dep of dependencies) {
                dep.changed(parameter);
            }
        }
    }

    beginEdit(tag: number): Promise<void> {
        return beginEdit(tag);
    }

    performEdit(tag: number, value: number): Promise<void> {
        return performEdit(tag, value);
    }

    endEdit(tag: number): Promise<void> {
        return endEdit(tag);
    }

    setParamNormalized(tag: number, value: number): Promise<void> {
        return setParamNormalized(tag, value).then( () =>
            getParameterObject(tag).then( (p) =>
                this.notifyParameterChange(p)
            )
        );
    }

    getParameterObject(tag: number): Promise<IParameter> {
        return getParameterObject(tag);
    }

    getParameterObjects(tags: Array<number>): Promise<{ [key: number]: IParameter }> {
        return getParameterObjects(tags)
    };

    getSelectedUnit(): Promise<number> {
        return getSelectedUnit();
    }

    selectUnit(unitId: number): Promise<void> {
        return selectUnit(unitId);
    }
}

export const controller = new Controller;

export {}

declare global {
    function notifyParameterChange(parameter :IParameter) : void;
}

const _global = window;
_global.notifyParameterChange = function (parameter :IParameter) : void {
    controller.notifyParameterChange(parameter);
}
