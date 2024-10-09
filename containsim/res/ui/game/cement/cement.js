class Concretion {
    /** @param {Concrete} element */
    constructor(element) {
        this.element = element;
    }
}

/** @type {Map<string, Concretion[]>} */
let concretions = new Map();

class Concrete extends HTMLElement {
    constructor() {
        super();

        const data_source = this.getAttribute("data-source");

        if (data_source == null && typeof data_source === "string" && data_source.length > 0)
            throw "data-source string attribute of non-zero length required for concrete element";
        
        if (concretions.has(data_source))
            concretions.get(data_source).push(new Concretion(this));
        else
            concretions.set(data_source, [new Concretion(this)]);
    }

    connectedCallback() {
        let aggregates = this.querySelectorAll("[aggr]");
        this._aggregates = aggregates;
    }

    /**
     * @param {string} data_source_name
     * @param {Object} value
     */
    updateFromDataSource(data_source_name, value) {
        for (let aggr of this._aggregates) {
            const field = aggr.getAttribute("aggr");

            if (field.length == 0)
                continue;

            const replacement = value[field];
            if (replacement === undefined)
                throw `Data source "${data_source_name}" missing field "${field}".`;

            aggr.innerHTML = replacement;
        }
    }

    /**
     *  @param {string} data_source 
     *  @param {Object} value
     */
    static updateDataSource(data_source, value) {
        if (!concretions.has(data_source))
            return;

        for (let concretion of concretions.get(data_source)) {
            concretion.element.updateFromDataSource(data_source, value);
        }
    }
}

window.onload = () => {
    customElements.define("concrete-mixer", Concrete);
}