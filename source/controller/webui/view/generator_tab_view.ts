import {addKnob, IParameterControl, Toggle} from "./controls";
import {MakeTab} from "./templates";
import {SelectedGeneratorDelegate, View} from "./views";
import * as Model from "../model/sidebands_model";

export class GeneratorTabView implements View {
    readonly element: HTMLElement;

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

