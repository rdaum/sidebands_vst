import {addKnob, IParameterControl} from "./controls";
import * as VstModel from "../model/vst_model";
import * as Model from "../model/sidebands_model";
import {ParamIDFor, ParamTag, TargetTag} from "../model/sidebands_model";
import * as Env from "./envelope_editor_view";
import {GD, GeneratorView, View} from "./views";
import {addTab, GeneratorTabView} from "./generator_tab_view";
import * as Viz from './harmonics_analysis_view';
import {Switch} from "./switch";

export class MainView implements View {
    controls: Array<IParameterControl>;
    subViews: Array<GeneratorView>;

    constructor() {
        this.controls = [];
        this.subViews = [];
    }

    build() {
        const selector_area = GD('generator_selector');
        let generators: Array<GeneratorTabView> = [];
        for (let x = 0; x < Model.kNumGenerators; x++) {
            let tab = addTab(selector_area, x, (selected: number) => {
                VstModel.controller.selectUnit(selected);
                this.updateSelectedGenerator(selected);
            });
            if (tab) {
                generators.push(tab);
            }
        }

        let g_hviz_area = GD("global-harmonics-visual");
        if (g_hviz_area)
            this.subViews.push(
                new Viz.HarmonicsAnaylsisView(<HTMLDivElement>g_hviz_area, -1,
                    "kRequestAnalysisBufferMessageID", "kResponseAnalysisBufferMessageID",
                    64, "Waveform")
            );
        let g_sviz_area = GD("global-spectrum-visual");
        if (g_sviz_area)
            this.subViews.push(
                new Viz.HarmonicsAnaylsisView(<HTMLDivElement>g_sviz_area, -1,
                    "kRequestSpectrumBufferMessageID", "kResponseSpectrumBufferMessageID",
                    256, "Spectrum")
            );


        VstModel.controller.getSelectedUnit().then(selectedUnit => {
            generators[selectedUnit].select();

            let pushControl = (c: IParameterControl) => {
                this.controls.push(c);
            };

            this.subViews.push(
                new Switch(<HTMLImageElement>GD('a_switch'), {
                    Generator: selectedUnit,
                    Param: Model.ParamTag.TAG_GENERATOR_TOGGLE,
                    Target: TargetTag.TARGET_NA
                }));
            addKnob(GD('a_level'),
                {
                    Generator: selectedUnit,
                    Param: Model.ParamTag.TAG_OSC,
                    Target: Model.TargetTag.TARGET_A
                }).then(pushControl);
            addKnob(GD('velsense'),
                {
                    Generator: selectedUnit,
                    Param: Model.ParamTag.TAG_ENV_VS,
                    Target: Model.TargetTag.TARGET_A
                }).then(pushControl);
            addKnob(GD('carrier_ratio'),
                {
                    Generator: selectedUnit,
                    Param: Model.ParamTag.TAG_OSC,
                    Target: Model.TargetTag.TARGET_C
                }).then(pushControl);
            addKnob(GD('modulation_ratio'),
                {
                    Generator: selectedUnit,
                    Param: Model.ParamTag.TAG_OSC,
                    Target: Model.TargetTag.TARGET_M
                }).then(pushControl);
            addKnob(GD('modulation_index'),
                {
                    Generator: selectedUnit,
                    Param: Model.ParamTag.TAG_OSC,
                    Target: Model.TargetTag.TARGET_K
                }).then(pushControl);

            let hviz_area = GD("harmonics-visual");
            if (hviz_area)
                this.subViews.push(
                    new Viz.HarmonicsAnaylsisView(<HTMLDivElement>hviz_area, selectedUnit,
                        "kRequestAnalysisBufferMessageID", "kResponseAnalysisBufferMessageID",
                        64, "Waveform")
                );
            let sviz_area = GD("spectrum-visual");
            if (sviz_area)
                this.subViews.push(
                    new Viz.HarmonicsAnaylsisView(<HTMLDivElement>sviz_area, selectedUnit,
                        "kRequestSpectrumBufferMessageID", "kResponseSpectrumBufferMessageID",
                        256, "Spectrum")
                );

            let a_env_area = GD("a_env_area");
            if (a_env_area)
                this.subViews.push(
                    new Env.EnvelopeEditorKnobView(<HTMLDivElement>a_env_area, selectedUnit, Model.TargetTag.TARGET_A));

            VstModel.controller.getParameterObject(ParamIDFor({
                Generator: selectedUnit, Param: ParamTag.TAG_MODULATIONS,
                Target: TargetTag.TARGET_NA
            })).then((p) => {

            });

            let k_env_area = GD("k_env_area");
            if (k_env_area)
                this.subViews.push(
                    new Env.EnvelopeEditorKnobView(<HTMLDivElement>k_env_area, selectedUnit, Model.TargetTag.TARGET_K));
            let a_env_graph_area = GD("a_env-graphical");
            if (a_env_graph_area)
                this.subViews.push(
                    new Env.GraphicalEnvelopeEditorView(<HTMLDivElement>a_env_graph_area, selectedUnit, Model.TargetTag.TARGET_A));
            let k_env_graph_area = GD("k_env-graphical");
            if (k_env_graph_area)
                this.subViews.push(
                    new Env.GraphicalEnvelopeEditorView(<HTMLDivElement>k_env_graph_area, selectedUnit, Model.TargetTag.TARGET_K));

        })

    }

    updateSelectedGenerator(gennum: number) {
        for (let control of this.controls) {
            control.updateSelectedGenerator(gennum);
        }
        for (let subView of this.subViews) {
            subView.updateSelectedGenerator(gennum);
        }
    }

    node(): HTMLElement {
        return <HTMLElement>GD("sidebands_view");
    }
}


