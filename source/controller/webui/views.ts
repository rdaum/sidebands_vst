import {MakeTab} from "./templates";
import * as Model from "./sidebandsModel";
import {addKnob, ParameterControl, Toggle} from "./controls";
import {IParameter} from "./sidebandsModel";

// Exported EditController functions.
export declare function beginEdit(tag: number): Promise<void>;
export declare function performEdit(tag: number, value: number): Promise<void>;
export declare function endEdit(tag: number): Promise<void>;
export declare function setParamNormalized(tag: number, value: number): Promise<void>;
export declare function getParameterObject(tag: number): Promise<IParameter>;
export declare function getParameterObjects(tag: Array<number>): Promise<{ [key: number]: IParameter }>;
export declare function getSelectedUnit(): Promise<number>;
export declare function selectUnit(unitId: number): Promise<void>;

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
    controls:  Array<ParameterControl>;
}

export class GeneratorTabView implements View {
    readonly element: HTMLElement;
    toggle: Toggle;
    controls: Array<ParameterControl>;

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

export class MainView implements View {
    controls: Array<ParameterControl>;

    constructor() {
        this.controls = [];
    }

    build() {
        const selector_area = GD('generator_selector');
        let generators: Array<GeneratorTabView> = [];
        for (let x = 0; x < 12; x++) {
            let tab = addTab(selector_area, x, (selected: number) => {
                selectUnit(selected);
            });
            if (tab)
                generators.push(tab);
        }

        getSelectedUnit().then(selectedUnit => {
            generators[selectedUnit].select();

            let pushControl = (c: ParameterControl) => {
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

            this.makeEnvelopeKnobs('a', selectedUnit, Model.TargetTag.TARGET_A);
            this.makeEnvelopeKnobs('k', selectedUnit, Model.TargetTag.TARGET_K);
        })

    }

    makeEnvelopeKnobs(elemPrefix: string, gennum: number, target: Model.TargetTag) {
        let pushControl = (c: ParameterControl) => {
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
}

