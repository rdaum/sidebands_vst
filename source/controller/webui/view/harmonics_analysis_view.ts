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

    constructor(readonly element: HTMLDivElement, private gennum: number) {
        console.log(element);
        element.appendChild(MakeHarmonicsView());
        this.canvas = element.querySelector('.graph-harmonics-canvas');
        for (const param of kHarmonicParams) {
            controller.subscribeParameter(ParamIDFor({
                Generator: gennum, Param: ParamTag.TAG_OSC,
                Target: param
            }), this);
        }

        controller.subscribeMessage("kResponseAnalysisBufferMessageID", this);
        this.refresh();
    }

    notify(messageId: string, message: Message): void {
        if (!this.canvas) return;
        let ctx = this.canvas.getContext("2d");
        if (!ctx) return;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        ctx.beginPath();
        ctx.lineWidth = 3;
        ctx.strokeStyle = "#1e2a96";
        ctx.moveTo(0, this.canvas.height);
        const msg = <AnalysisBufferMessage>message;
        const scalefactor = this.canvas.width / msg.kResponseAnalysisBufferSize;
        for (let i = 0; i < msg.kResponseAnalysisBufferData.length; i++) {
            const x = i * scalefactor;
            const y = msg.kResponseAnalysisBufferData[i] * this.canvas.height / 2;
            ctx.lineTo(x, (this.canvas.height / 2) + y);
        }
        ctx.stroke();
    }

    refresh() {
        controller.sendMessage("kRequestAnalysisBufferMessageID",
            {kRequestAnalysisBufferSampleRate: 32768,
                kRequestAnalysisBufferGennum: this.gennum,
                kRequestAnalysisBufferSize: 1024,
                kRequestAnalysisBufferFreq: 64});
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
