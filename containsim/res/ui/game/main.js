function horizontalScroll(sensitivity, event) {
    if (!event.deltaY) {
        return;
    }

    console.log("Scroll!");

    event.currentTarget.scrollLeft += sensitivity * (-event.deltaY + event.deltaX);
    event.preventDefault();
}

window.onload = (event) => {
    console.log("Page is fully loaded!");
    
    let hgrid_elements = document.getElementsByClassName("hgrid");

    Array.from(hgrid_elements).forEach(function(element) {
        
        let sensitivity = parseFloat(element.dataset.scrollSensitivity);
        console.log("Adding scroll event with sensitivity " + toString(sensitivity));
        element.addEventListener("wheel", horizontalScroll.bind(null, sensitivity));
    });
}