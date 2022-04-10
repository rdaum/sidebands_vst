
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

export declare function sendMessage(messageId: string, message : {[attrId: string]: any}) : Promise<void>;

export interface IDependent {
    changed(parameter : IParameter) : void;
}
type ParamDependencies = {[tag: number]: Array<IDependent>};

export type Message =  {[attrId: string]: any};
export interface IMsgSubscriber {
    notify(messageId: string, message: Message): void;
}
type MsgSubscribers = {[msgId: string]: Array<IMsgSubscriber>};

export class Controller {
    private dependencies : ParamDependencies;
    private subscribers : MsgSubscribers;
    
    constructor() {
        this.dependencies = {};
        this.subscribers = {};
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

    subscribeMessage(msgId: string, subscriber :IMsgSubscriber) {
        if (!this.subscribers[msgId]) this.subscribers[msgId] = [];
        this.subscribers[msgId].push(subscriber);
    }

    sendMessage(messageId: string, message : Message) : Promise<void> {
        return sendMessage(messageId, message);
    }
    
    receiveMessage(message: Message) {
        const messageId = message.messageId;
        if (!messageId) return;
        const subs = this.subscribers[messageId];
        for (const sub of subs) {
            sub.notify(messageId, message);
        }
    }
}

export const controller = new Controller;

export {}


declare global {
    function notifyParameterChange(parameter :IParameter) : void;
    function receiveMessage(payload: Message) : void;
}

const _global = window;
_global.notifyParameterChange = function (parameter :IParameter) : void {
    controller.notifyParameterChange(parameter);
}

_global.receiveMessage = function(payload : Message) : void {
    controller.receiveMessage(payload);
}
