export class BackButtonComponent {
    constructor(parent) { this.parent = parent; }
    getHTML() { return `<button id="back-button" class="btn btn-outline-red mb-4 ms-3 mt-3"><i class="fas fa-arrow-left me-2"></i>На главную</button>`; }
    addListeners(listener) { document.getElementById("back-button")?.addEventListener("click", listener); }
    render(listener) { this.parent.insertAdjacentHTML('afterbegin', this.getHTML()); this.addListeners(listener); }
}
