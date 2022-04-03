// Shortcut to access dom element.
export function GD(id: string): HTMLElement | null {
    const el = document.getElementById(id);
    if (!el) {
        console.log(`Unable to find: ${id}`);
    }
    return el;
}

export type SelectedGeneratorDelegate = (unitId: number) => void;

export interface View {
    node() : HTMLElement;
}

export interface GeneratorView extends View {
   updateSelectedGenerator(gennum: number): void;
}

