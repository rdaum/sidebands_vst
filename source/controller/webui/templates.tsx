import * as elements from 'typed-html';
import {Attributes, CustomElementHandler} from "typed-html"

function Tab(attributes : Attributes, contents: string[]) {
    let gennum_str = attributes['generator-number'];
    let gennum = parseInt(gennum_str.toString());
    return <span id={`generator_${gennum}`} is-selected='false' class='tabs-span tab-inactive'>
        <div class='tabs-label-div'>
            <label class='tabs-label'>#{gennum + 1}</label>
            <input class='label-toggle' type='checkbox' id={`generator_${gennum}_toggle`}/>
        </div>
        <div id={`generator_${gennum}_level`}>
        </div>
    </span>;
}

function MakeElement<T extends HTMLElement>(html : string) : HTMLElement {
    let t = document.createElement('template');
    t.insertAdjacentHTML("afterbegin", html);
    return t.firstChild as HTMLElement;
}

export function MakeTab(gennum : number) : HTMLElement {
    return MakeElement(<Tab generator-number={gennum}></Tab>);
}