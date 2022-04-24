import * as Model from "../model/sidebands_model";
import {AnalysisBufferMessage, kNumGenerators, ParamIDFor, ParamTag} from "../model/sidebands_model";
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
    private font : FontFace | null = null;

    constructor(readonly element: HTMLDivElement, private gennum: number,
                readonly requestMsgId: string, readonly responseMsgId : string, readonly frequency : number,
                readonly title : string) {

        const st_font = new FontFace('atari_st', 'url(//db.onlinewebfonts.com/t/7a7d5578bd2ddba4a33b77d1f90fa994.woff2)');
        st_font.load().then((font) => {
            document.fonts.add(font);
            this.font = font;
            console.log(`Font ${this.font.family} loaded: ${this.font.loaded}`);
            this.refresh();
        });

        console.log(element);
        element.appendChild(MakeHarmonicsView());
        this.canvas = element.querySelector('.graph-harmonics-canvas');
        for (const param of kHarmonicParams) {
            for (let g = 0; g < kNumGenerators; g++) {
                controller.subscribeParameter(ParamIDFor({
                    Generator: g, Param: ParamTag.TAG_OSC,
                    Target: param
                }), this);
            }
        }

        controller.subscribeMessage(this.responseMsgId, this);
        this.refresh();
    }

    notify(messageId: string, message: Message): void {
        const msg = <AnalysisBufferMessage>message;
        if (msg.gennum != this.gennum) return;
        if (!this.canvas) return;
        let ctx = this.canvas.getContext("2d");
        if (!ctx) return;
        ctx.imageSmoothingEnabled = false;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = "#1e2a96";
        ctx.moveTo(0, this.canvas.height);
        const scalefactor = this.canvas.width / msg.bufferSize;
        for (let i = 0; i < msg.bufferData.length; i++) {
            const x = i * scalefactor;
            const y = msg.bufferData[i] * this.canvas.height / 2;
            ctx.lineTo(x, (this.canvas.height / 2) - y);
        }
        ctx.stroke();
        ctx.font = "16px atari_st";
        ctx.fillStyle = "#1e2a96";
        ctx.fillText(this.title, this.canvas.width - ctx.measureText(this.title).width - 12, 20);
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
        if (this.gennum == -1) return;
        this.gennum = gennum;
        this.refresh();
    }
}
