import * as elements from 'typed-html';
import {Attributes, CustomElementHandler} from "typed-html"

function MakeElement<T extends HTMLElement>(html : string) : HTMLElement {
    let t = document.createElement('template');
    t.insertAdjacentHTML("afterbegin", html);
    return t.firstChild as HTMLElement;
}

function Tab(attributes : Attributes, contents: string[]) {
    let gennum_str = attributes['generator-number'];
    let gennum = parseInt(gennum_str.toString());
    return <span id={`generator_${gennum}`} is-selected='false' class='tabs-span tab-inactive'>
        <div class='tabs-label-div'>
            <label class='tabs-label'>{gennum + 1}</label>
        </div>
    </span>;
}

export function MakeTab(gennum : number) : HTMLElement {
    return MakeElement(<Tab generator-number={gennum}></Tab>);
}

function EnvelopeKnobs(attributes : Attributes, contents: string[]) {
    let target_str = attributes['target-prefix'];
    return <div class="envelope-editor editor-panel" id={target_str + "_env_editor"}>
        <div class="parameter hold">
            <label>Hold</label>
            <div id={target_str + "_hold_time"}></div>
        </div>
        <span>&nbsp;</span>
        <div class="parameter a-rate">
            <label>Attack</label>
            <div id={target_str + "_attack"}></div>
        </div>
        <span>&nbsp;</span>
        <div class="parameter d1-rate">
            <label>Decay1 Time</label>
            <div id={target_str + "_d1_r"}></div>
        </div>
        <div class="parameter d2-rate">
            <label>Decay2 Time</label>
            <div id={target_str + "_d2_r"}></div>
        </div>
        <div class="parameter r1-rate">
            <label>Release1 Time</label>
            <div id={target_str + "_r1_r"}></div>
        </div>
        <div class="parameter r2-date">
            <label>Release2 Time</label>
            <div id={target_str + "_r2_r"}></div>
        </div>

        <div class="parameter a-level">
            <label>Peak</label>
            <div id={target_str + "_attack_level"}></div>
        </div>
        <div class="parameter d1-level">
            <label>Decay1 Level</label>
            <div id={target_str + "_d1_l"}></div>
        </div>
        <div class="parameter s-level">
            <label>Sustain Level</label>
            <div id={target_str + "_s_l"}></div>
        </div>
        <div class="parameter r-level">
            <label>Release Level</label>
            <div id={target_str + "_r1_l"}></div>
        </div>
    </div>;
}

export function MakeEnvelopeEditor(targetPrefix : string) {
    return MakeElement(<EnvelopeKnobs target-prefix={targetPrefix}/>);
}

function GraphicalEnvelope(attributes : Attributes, contents: string[]) {
    let target_str = attributes['target-prefix'];
    return <div id={target_str + "-graph-env"} class="graphical-env-container">
        <canvas id={target_str + "-graph-env-canvas"} class="viz-canvas"></canvas>
    </div>;
}

export function MakeGraphicalEnvelopeEditor(targetPrefix : string) {
    return MakeElement(<GraphicalEnvelope target-prefix={targetPrefix}/>);
}

function HarmonicsView(attributes : Attributes, contents: string[]) {
    return <canvas class={"graph-harmonics-canvas viz-canvas"}></canvas>;
}

export function MakeHarmonicsView() {
    return MakeElement(<HarmonicsView/>);
}

