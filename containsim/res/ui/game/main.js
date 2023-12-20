function HorizontalScroll(sensitivity, event) {
    if (!event.deltaY) {
        return;
    }

    console.log("Scroll!");

    event.currentTarget.scrollLeft += sensitivity * (-event.deltaY + event.deltaX);
    event.preventDefault();
}

function OnButtonClickCallFunc(func, event) {
    func(event.target);
    event.stopPropagation();
}

window.addEventListener("load", (event) => {
    console.log("Page is fully loaded!");
    
    let hgrid_elements = document.getElementsByClassName("hgrid");
    console.log(hgrid_elements);

    Array.from(hgrid_elements).forEach((hgrid) => {
        console.log(hgrid.outerHTML);

        let sensitivity = parseFloat(hgrid.dataset.scrollSensitivity);
        console.log("Adding scroll event with sensitivity " + toString(sensitivity));
        hgrid.addEventListener("wheel", HorizontalScroll.bind(null, sensitivity));

        let items = hgrid.getElementsByClassName("item");
        let itemSelectFunc = hgrid.dataset.itemSelectFunc;
        
        Array.from(items).forEach((item) => {
            item.addEventListener("click", OnButtonClickCallFunc.bind(null, window[itemSelectFunc]));
        });

    });
});

function SelectTile(element) {
    console.log(element.outerHtml);
    OutputCommand();
}