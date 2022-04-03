import {addKnob, IParameterControl, Toggle} from "./controls";
import {MakeTab} from "./templates";
import {SelectedGeneratorDelegate, View} from "./views";
import * as Model from "../model/sidebands_model";

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

export function addTab(parent: HTMLElement | null, gennum: number, selectedDelegate: SelectedGeneratorDelegate): GeneratorTabView | null {
    if (!parent) return null;
    return new GeneratorTabView(gennum, selectedDelegate, parent);
}

