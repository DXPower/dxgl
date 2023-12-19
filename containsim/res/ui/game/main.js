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
}

window.addEventListener("load", (event) => {
    console.log("Page is fully loaded!");
    
    let hgrid_elements = document.getElementsByClassName("hgrid");

    Array.from(hgrid_elements).forEach((element) => {
        let sensitivity = parseFloat(element.dataset.scrollSensitivity);
        console.log("Adding scroll event with sensitivity " + toString(sensitivity));
        element.addEventListener("wheel", HorizontalScroll.bind(null, sensitivity));

        let elemSelectFunc = element.dataset.elemSelectFunc;
        element.addEventListener("click", OnButtonClickCallFunc.bind(null, window[elemSelectFunc]));
    });
});

function SelectTile(element) {
    OutputCommand();
}