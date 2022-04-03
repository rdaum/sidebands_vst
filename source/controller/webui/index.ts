import {MainView} from './view/main_view';

export function ready() {
    console.log("Awoken.");

    const view = new MainView();
    view.build();
}

document.addEventListener('DOMContentLoaded', ready, false);
