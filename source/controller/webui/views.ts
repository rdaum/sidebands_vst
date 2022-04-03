// Where composite views go

import {MakeEnvelopeEditor, MakeTab} from "./templates";
import * as Model from "./sidebandsModel";
import {addKnob, IParameterControl, Toggle} from "./controls";
import * as VstModel from "./vstModel";

// Shortcut to access dom element.
function GD(id: string): HTMLElement | null {
    const el = document.getElementById(id);
    if (!el) {
        console.log(`Unable to find: ${id}`);
    }
    return el;
}

type SelectedGeneratorDelegate = (unitId: number) => void;

interface View {
    node() : HTMLElement;
}

interface GeneratorView extends View {
   updateSelectedGenerator(gennum: number): void;
}

export class GeneratorTabView implements View {
    readonly element: HTMLElement;
    toggle: Toggle;
    controls: Array<IParameterControl>;

    constructor(readonly gennum: number, readonly selectDelegate: SelectedGeneratorDelegate, parent: HTMLElement) {
        this.gennum = gennum;

        // Make the HTML for us, add our listener, and stick it onto the parent.
        const rootTabElement = MakeTab(gennum);
        rootTabElement.addEventListener("click", () => {
            this.select();
            this.selectDelegate(this.gennum);
        })
        this.element = rootTabElement;
        parent.appendChild(this.element);

        // Now activate/build controls that live inside our HTML.
        const toggleInput = this.element.querySelector(`#generator_${gennum}_toggle`);
        this.toggle = new Toggle({
            Generator: this.gennum,
            Param: Model.ParamTag.TAG_GENERATOR_TOGGLE,
            Target: Model.TargetTag.TARGET_NA
        }, <HTMLInputElement>toggleInput);

        this.controls = [this.toggle];

        const knobElement = this.element.querySelector(`#generator_${gennum}_level`);
        addKnob(knobElement, {
            Generator: this.gennum,
            Param: Model.ParamTag.TAG_OSC,
            Target: Model.TargetTag.TARGET_A
        }).then((c) =>
            this.controls.push(c)
        )
    }

    select() {
        let element = this.element;

        const selected = element.getAttribute('is-selected');
        if (selected != "false") return;

        element.setAttribute('is-selected', 'true');
        element.classList.replace('tab-inactive', 'tab-active');

        let parent = element.parentNode;
        if (!parent) return;
        for (let tabSpan of parent.children) {
            if (tabSpan == element) {
                continue;
            }
            const childSelected = tabSpan.getAttribute('is-selected');
            if (childSelected == "false") {
                continue;
            }
            tabSpan.setAttribute('is-selected', "false");
            tabSpan.classList.replace('tab-active', 'tab-inactive');
        }
    }

    node(): HTMLElement {
        return this.element;
    }
}

function addTab(parent: HTMLElement | null, gennum: number, selectedDelegate: SelectedGeneratorDelegate): GeneratorTabView | null {
    if (!parent) return null;

    let tab = new GeneratorTabView(gennum, selectedDelegate, parent);

    return tab;
}

export class EnvelopeEditorKnobView implements GeneratorView {
    controls: Array<IParameterControl>;

    constructor(readonly element :HTMLDivElement, readonly gennum : number, target : Model.TargetTag) {
        this.controls = [];
        const targetPrefix = target.toString().toLowerCase();
        let panel = MakeEnvelopeEditor(targetPrefix);
        element.appendChild(panel);

        this.makeEnvelopeKnobs(targetPrefix, gennum, target);
    }

    updateSelectedGenerator(gennum: number): void {
        for (let control of this.controls) {
            control.updateSelectedGenerator(gennum);
        }
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
        for (let x = 0; x < 12; x++) {
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

            let a_env_area = GD("a_env_area");
            if (a_env_area)
                this.subViews.push(new EnvelopeEditorKnobView(<HTMLDivElement>a_env_area, selectedUnit, Model.TargetTag.TARGET_A));
            let k_env_area = GD("k_env_area");
            if (k_env_area)
                this.subViews.push(new EnvelopeEditorKnobView(<HTMLDivElement>k_env_area, selectedUnit, Model.TargetTag.TARGET_K));
        })

    }

    updateSelectedGenerator(gennum : number) {
        for (let control of this.controls) {
            control.updateSelectedGenerator(gennum);
        }
        for (let subView of this.subViews) {
            subView.updateSelectedGenerator(gennum);
        }
    }

    node() : HTMLElement {
        return <HTMLElement>GD("sidebands_view");
    }
}


