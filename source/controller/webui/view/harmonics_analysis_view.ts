import * as Model from "../model/sidebands_model";
import {AnalysisBufferMessage, ParamIDFor, ParamTag} from "../model/sidebands_model";
import {MakeHarmonicsView} from "./templates";
import {GeneratorView} from "./views";
import {controller, IDependent, IMsgSubscriber, IParameter, Message} from "../model/vst_model";

const kHarmonicParams = [
    Model.TargetTag.TARGET_K,
    Model.TargetTag.TARGET_C,
    Model.TargetTag.TARGET_C,
    Model.TargetTag.TARGET_R,
    Model.TargetTag.TARGET_S,
    Model.TargetTag.TARGET_M
];

export class HarmonicsAnaylsisView implements GeneratorView, IDependent, IMsgSubscriber {
    private canvas: HTMLCanvasElement | null;

    constructor(readonly element: HTMLDivElement, private gennum: number,
                readonly requestMsgId: string, readonly responseMsgId : string, readonly frequency : number,
                readonly title : string) {
        console.log(element);
        element.appendChild(MakeHarmonicsView());
        this.canvas = element.querySelector('.graph-harmonics-canvas');
        for (const param of kHarmonicParams) {
            controller.subscribeParameter(ParamIDFor({
                Generator: gennum, Param: ParamTag.TAG_OSC,
                Target: param
            }), this);
        }

        controller.subscribeMessage(this.responseMsgId, this);
        this.refresh();
    }

    notify(messageId: string, message: Message): void {
        if (!this.canvas) return;
        let ctx = this.canvas.getContext("2d");
        if (!ctx) return;
        ctx.imageSmoothingEnabled = false;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = "#1e2a96";
        ctx.moveTo(0, this.canvas.height);
        const msg = <AnalysisBufferMessage>message;
        const scalefactor = this.canvas.width / msg.bufferSize;
        for (let i = 0; i < msg.bufferData.length; i++) {
            const x = i * scalefactor;
            const y = msg.bufferData[i] * this.canvas.height / 2;
            ctx.lineTo(x, (this.canvas.height / 2) - y);
        }
        ctx.stroke();
        ctx.font = "atari_st";
        ctx.strokeText(this.title, this.canvas.width - ctx.measureText(this.title).width - 12, 12);
    }

    refresh() {
        controller.sendMessage(this.requestMsgId,
            {
                sampleRate: 32768,
                gennum: this.gennum,
                bufferSize: 1024,
                frequency: this.frequency
            });
    }

    node(): HTMLElement {
        return this.element;
    }

    changed(parameter: IParameter): void {
        this.refresh();
    }

    updateSelectedGenerator(gennum: number): void {
        this.gennum = gennum;
        this.refresh();
    }
}
