import {addKnob, IParameterControl} from "./controls";
import * as Model from "../model/sidebands_model";
import {ParamTag, ParseTag, TargetTag} from "../model/sidebands_model";
import {MakeEnvelopeEditor, MakeGraphicalEnvelopeEditor} from "./templates";
import {GD, GeneratorView} from "./views";
import {controller, IDependent, IParameter, IRangeParameter, ValueOf} from "../model/vst_model";


export class EnvelopeEditorKnobView implements GeneratorView {
    controls: Array<IParameterControl>;

    constructor(readonly element: HTMLDivElement, private gennum: number, target: Model.TargetTag) {
        this.controls = [];

        const targetPrefix = Model.TargetTag[target].toString().toLowerCase();
        let panel = MakeEnvelopeEditor(targetPrefix);
        element.appendChild(panel);

        this.makeEnvelopeKnobs(targetPrefix, gennum, target);
    }

    updateSelectedGenerator(gennum: number): void {
        for (let control of this.controls) {
            control.updateSelectedGenerator(gennum);
        }
        this.gennum = gennum;
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

/*
  struct Segment {
    Steinberg::Vst::RangeParameter *rate_param;
    Steinberg::Vst::RangeParameter *start_level_param;
    Steinberg::Vst::RangeParameter *end_level_param;

    VSTGUI::CCoord width;
    VSTGUI::CPoint start_point;
    VSTGUI::CPoint end_point;
    VSTGUI::CRect drag_box;
  };

 */

interface SegmentParams {
    rateParam: IRangeParameter | null;
    startLevelParam: IRangeParameter | null;
    endLevelParam: IRangeParameter | null;
}

type Point = [number, number];
type Rect = [number, number, number, number];

interface SegmentPoints {
    width: number;
    startPoint: Point;
    endPoint: Point;
    dragBox: Rect | null;
}

interface Segment {
    id: number;
    params: SegmentParams;
    points: SegmentPoints | null;
}

const kSustainDuration = 0.25;
const kDragboxWidthHeight = 8;
const kDragboxHalfWidthHeight = kDragboxWidthHeight / 2;

function EnvelopeRampCoefficient(start_level: number, end_level: number, length_in_samples: number) {
    return 1.0 + (Math.log(end_level) - Math.log(start_level)) / length_in_samples;
}


interface SegmentDecl {
    rateParam: Model.ParamTag | null;
    startLevelParam: Model.ParamTag | null;
    endLevelParam: Model.ParamTag | null;
}

const kSegmentDecls: { [id: number]: SegmentDecl } = {
    0: {rateParam: Model.ParamTag.TAG_ENV_HT, startLevelParam: null, endLevelParam: null},
    1: {rateParam: Model.ParamTag.TAG_ENV_AR, startLevelParam: null, endLevelParam: ParamTag.TAG_ENV_AL},
    2: {
        rateParam: Model.ParamTag.TAG_ENV_DR1,
        startLevelParam: ParamTag.TAG_ENV_AL,
        endLevelParam: ParamTag.TAG_ENV_DL1
    },
    3: {
        rateParam: Model.ParamTag.TAG_ENV_DR2,
        startLevelParam: ParamTag.TAG_ENV_DL1,
        endLevelParam: ParamTag.TAG_ENV_SL
    },
    4: {rateParam: null, startLevelParam: Model.ParamTag.TAG_ENV_SL, endLevelParam: ParamTag.TAG_ENV_SL},
    5: {
        rateParam: Model.ParamTag.TAG_ENV_RR1,
        startLevelParam: ParamTag.TAG_ENV_SL,
        endLevelParam: ParamTag.TAG_ENV_RL1
    },
    6: {rateParam: Model.ParamTag.TAG_ENV_RR2, startLevelParam: ParamTag.TAG_ENV_RL1, endLevelParam: null},
}

type SegmentList = { [index: number]: Segment };

export class GraphicalEnvelopeEditorView implements GeneratorView, IDependent {
    private segments: SegmentList;
    private canvas: HTMLCanvasElement | null;
    private draggingSegment: Segment | null;

    constructor(readonly element: HTMLDivElement, private gennum: number, readonly target: Model.TargetTag) {
        const targetPrefix = Model.TargetTag[target].toString().toLowerCase();

        element.appendChild(MakeGraphicalEnvelopeEditor(targetPrefix));
        this.canvas = element.querySelector(`#${targetPrefix}-graph-env-canvas`);
        this.segments = {};
        this.draggingSegment = null;

        // // Subscribe for parameter changes for all envelope parameters.
        for (const param of [Model.ParamTag.TAG_ENV_HT, Model.ParamTag.TAG_ENV_AR, ParamTag.TAG_ENV_AL,
            Model.ParamTag.TAG_ENV_DR1, ParamTag.TAG_ENV_DL1,
            ParamTag.TAG_ENV_DR2,
            ParamTag.TAG_ENV_SL,
            ParamTag.TAG_ENV_RR1,
            ParamTag.TAG_ENV_RL1,
            ParamTag.TAG_ENV_RR2,]) {
            controller.subscribeParameter(Model.ParamIDFor({
                Generator: this.gennum,
                Param: param,
                Target: target
            }), this);
        }

        if (this.canvas) {
            this.canvas.addEventListener('mousedown', e => {
                this.onMouseDown(e);
            })
            this.canvas.addEventListener('mousemove', e => {
                this.onMouseMove(e);
            })
            this.canvas.addEventListener('mouseup', e => {
                this.onMouseUp(e);
            })
        }
        this.refresh();
    }

    refresh(ids: Array<number> | null = null) {
        this.updateSegments(ids).then((segments) => {
            this.redraw();
        });
    }

    loadSegmentParams(index: number,
                      rateParam: Model.ParamTag | null,
                      startLevelParam: Model.ParamTag | null,
                      endLevelParam: Model.ParamTag | null): Promise<SegmentParams> {
        const promises = [
            rateParam != null ? controller.getParameterObject(Model.ParamIDFor({
                Generator: this.gennum,
                Param: rateParam,
                Target: this.target
            })) : null,
            startLevelParam != null ? controller.getParameterObject(Model.ParamIDFor({
                Generator: this.gennum,
                Param: startLevelParam,
                Target: this.target
            })) : null,
            endLevelParam != null ? controller.getParameterObject(Model.ParamIDFor({
                Generator: this.gennum,
                Param: endLevelParam,
                Target: this.target
            })) : null
        ]
        return new Promise((resolve, reject) => {
            const params = this.segments[index].params;
            Promise.all(promises).then((values) => {
                params.rateParam = <IRangeParameter>values[0];
                params.startLevelParam = <IRangeParameter>values[1];
                params.endLevelParam = <IRangeParameter>values[2];
                resolve(params);
            })
        })

    }

    emptySegment(id: number): Segment {
        return {
            id: id,
            params: {
                rateParam: null, endLevelParam: null, startLevelParam: null
            }, points: null
        };
    }

    updateSegments(updatedSegments: Array<number> | null = null): Promise<SegmentList> {
        if (!updatedSegments) updatedSegments = [0, 1, 2, 3, 4, 5, 6];
        let refreshPromises: Array<Promise<SegmentParams>> = [];
        for (const us of updatedSegments) {
            this.segments[us] = this.emptySegment(us);
            const sdecl = kSegmentDecls[us];
            refreshPromises.push(this.loadSegmentParams(
                us,
                sdecl.rateParam, sdecl.startLevelParam, sdecl.endLevelParam
            ))
        }

        // Once all segment params are loaded we can then proceed to do the calculations of points.
        return new Promise((resolve, reject) => {
            Promise.all(refreshPromises).then((segmentParams) => {
                let totalDuration = 0;
                for (const sd in this.segments) {
                    if (!this.segments[sd] || !this.segments[sd].params) continue;
                    const segmentParam = this.segments[sd].params;
                    if (!segmentParam.rateParam) continue;
                    if (Model.ParseTag(segmentParam.rateParam.info.id).Param == Model.ParamTag.TAG_ENV_SL) {
                        totalDuration += kSustainDuration;
                    } else {
                        totalDuration += ValueOf(segmentParam.rateParam);
                    }
                }
                if (this.canvas) {
                    let xpos = 0;
                    const height = this.canvas.height;
                    const width = this.canvas.width;
                    for (const s in this.segments) {
                        const segment = this.segments[s];
                        let duration = 0;
                        if (segment.params.rateParam) {
                            if (Model.ParseTag(segment.params.rateParam.info.id).Param == Model.ParamTag.TAG_ENV_SL) {
                                duration = kSustainDuration;
                            } else {
                                duration = ValueOf(segment.params.rateParam);
                            }
                        }
                        const start_level = ValueOf(segment.params.startLevelParam);
                        const end_level = ValueOf(segment.params.endLevelParam);
                        const yleft = height - (start_level * height);
                        const yright = height - (end_level * height);
                        const startPoint: Point = [xpos, yleft];
                        const swidth = (duration / totalDuration) * width;
                        xpos += swidth;
                        const endPoint: Point = [xpos, yright];

                        let dragBox: Rect | null = null;
                        if (segment.params.rateParam || segment.params.startLevelParam) {
                            dragBox = [
                                endPoint[0] - kDragboxHalfWidthHeight,
                                endPoint[1] - kDragboxHalfWidthHeight,
                                kDragboxWidthHeight, kDragboxWidthHeight
                            ]
                        }
                        segment.points = {
                            width: swidth,
                            startPoint: startPoint,
                            endPoint: endPoint,
                            dragBox: dragBox
                        }
                    }
                }
                resolve(this.segments);
            });
        });
    }

    onMouseDown(event: MouseEvent) {
        if (!this.draggingSegment) {
            for (const sid in this.segments) {
                const s = this.segments[sid];
                if (!s.points) continue;
                if (!s.points.dragBox) continue;
                if (event.offsetX >= s.points.dragBox[0] &&
                    event.offsetY >= s.points.dragBox[1] &&
                    event.offsetX <= s.points.dragBox[0] + s.points.dragBox[2] &&
                    event.offsetY <= s.points.dragBox[1] + s.points.dragBox[3]) {
                    this.draggingSegment = s;
                }
            }
        }
    }

    onMouseMove(event: MouseEvent) {
        if (!this.draggingSegment) return;
        if (!this.canvas) return;
        if (!this.draggingSegment.params || !this.draggingSegment.points || !this.draggingSegment.points.width) return;
        if (event.offsetX < 0 && event.offsetY < 0) return;
        if (event.offsetX > this.canvas.width || event.offsetY > this.canvas.height) return;
        if (!this.draggingSegment.points.dragBox) return;

        let changed = false;

        const deltaX = event.offsetX - this.draggingSegment.points.dragBox[0] + kDragboxHalfWidthHeight;
        const rateParam = this.draggingSegment.params.rateParam;
        if (rateParam && deltaX) {
            const deltaR = deltaX / this.draggingSegment.points.width;
            const newValue = Math.max(rateParam.normalized + deltaR, 0.0001);
            controller.setParamNormalized(rateParam.info.id, newValue);
            changed = true;
        }

        const levelParam = this.draggingSegment.params.endLevelParam;
        if (levelParam) {
            // float change_n = 1 - (y / getHeight());
            const deltaL = 1 - (event.offsetY / this.canvas.height);
            controller.setParamNormalized(levelParam.info.id, deltaL);
            changed = true;
        }
    }

    onMouseUp(event: MouseEvent) {
        this.draggingSegment = null;
    }

    node(): HTMLElement {
        return this.element;
    }

    changed(parameter: IParameter): void {
        let tag = ParseTag(parameter.info.id);
        if (tag.Generator == this.gennum && tag.Target == this.target) {
            let updatedSegments: Array<number> = [];
            for (const sd in this.segments) {
                const segment = this.segments[sd];
                if (!segment.params) continue;
                if (segment.params.rateParam?.info.id == parameter.info.id ||
                    segment.params.endLevelParam?.info.id == parameter.info.id ||
                    segment.params.startLevelParam?.info.id == parameter.info.id)
                    updatedSegments.push(segment.id);
            }
            this.refresh(updatedSegments);
        }
    }

    updateSelectedGenerator(gennum: number): void {
        this.gennum = gennum;
        this.refresh();
    }

    redraw() {
        if (!this.segments || !this.canvas) return;
        let ctx = this.canvas.getContext("2d");
        if (!ctx) return;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        ctx.beginPath();
        ctx.lineWidth = 3;
        ctx.strokeStyle = "#1e2a96";
        ctx.moveTo(0, this.canvas.height);
        let x = 0;
        for (const sid in this.segments) {
            const s = this.segments[sid];
            if (!s.points) continue;
            let level = ValueOf(s.params.startLevelParam);
            if (level == 0) level = 0.001;
            let end_level = ValueOf(s.params.endLevelParam);
            if (end_level == 0) end_level = 0.001;

            const co = EnvelopeRampCoefficient(level, end_level, s.points.width);
            for (let i = 0; i < s.points.width; i++) {
                level *= co;
                let height = level * this.canvas.height;
                ctx.lineTo(x++, this.canvas.height - height);
            }
        }
        ctx.stroke();
        ctx.lineWidth = 1;
        ctx.beginPath();
        for (const sid in this.segments) {
            const s = this.segments[sid];
            if ((s.params.rateParam || s.params.endLevelParam) && s.points?.dragBox) {
                ctx.rect(s.points.dragBox[0], s.points.dragBox[1], s.points.dragBox[2], s.points.dragBox[3]);
            }
        }

        ctx.stroke();
    }
}