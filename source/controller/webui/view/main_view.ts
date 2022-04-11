import {addKnob, IParameterControl} from "./controls";
import * as VstModel from "../model/vst_model";
import * as Model from "../model/sidebands_model";
import * as Env from "./envelope_editor_view";
import {GD, GeneratorView, View} from "./views";
import {addTab, GeneratorTabView} from "./generator_tab_view";
import * as Viz from './harmonics_analysis_view';

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

        VstModel.controller.getSelectedUnit().then(selectedUnit => {
            generators[selectedUnit].select();

            let pushControl = (c: IParameterControl) => {
                this.controls.push(c);
            };

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
                        64)
                );
            let sviz_area = GD("spectrum-visual");
            if (sviz_area)
                this.subViews.push(
                    new Viz.HarmonicsAnaylsisView(<HTMLDivElement>sviz_area, selectedUnit,
                        "kRequestSpectrumBufferMessageID", "kResponseSpectrumBufferMessageID",
                        256)
                );

            let a_env_area = GD("a_env_area");
            if (a_env_area)
                this.subViews.push(
                    new Env.EnvelopeEditorKnobView(<HTMLDivElement>a_env_area, selectedUnit, Model.TargetTag.TARGET_A));
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


