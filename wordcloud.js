function getWordCloudURL(topic, numWords, color) {
    var url = topic+".jpg";
    console.log(topic, numWords, color);
    return url;
}

function updateWordCloud() {
    var topic = document.getElementById("topics").value;
    // var shape = document.getElementById("shapes").value;
    var numWords = document.getElementById("num-words").value;
    var color = document.getElementById("color").checked;
    var img = document.getElementById('wordcloudimg');
    img.src = getWordCloudURL(topic, numWords, color);
    img.alt = "Word Cloud of " + topic;
}

function darkmode() {
    const prefersDarkScheme = window.matchMedia("(prefers-color-scheme: dark)");

    if (prefersDarkScheme.matches) {
        document.body.classList.toggle("light-theme");
    } else {
        document.body.classList.toggle("dark-theme");
    }
}

