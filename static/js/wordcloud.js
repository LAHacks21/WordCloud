function setWordCloudURL(topic, numWords, color) {
    var url = "img/"+topic+".jpg";
    axios.get('http://localhost:5000/cloud', {
        params: {
            topic: topic,
            numWords: numWords,
            color: color
        }
    }).then(function (response) {
        var img = document.getElementById('wordcloudimg');
        img.src = response.data;
        console.log(response);
    }).catch(function (error) {
        console.error(error);
    });
    console.log(topic, numWords, color);
    return url;
}

function updateWordCloud() {
    var topic = document.getElementById("topics").value;
    // var shape = document.getElementById("shapes").value;
    var numWords = document.getElementById("num-words").value;
    var color = document.getElementById("color").checked;
    var img = document.getElementById('wordcloudimg');
    setWordCloudURL(topic, numWords, color);
    img.alt = "Word Cloud of " + topic;
    var caption = document.getElementById('caption');
    caption.innerHTML = "Wordcloud of " + topic;
}

function darkmode() {
    const prefersDarkScheme = window.matchMedia("(prefers-color-scheme: dark)");

    if (prefersDarkScheme.matches) {
        document.body.classList.toggle("light-theme");
    } else {
        document.body.classList.toggle("dark-theme");
    }
}

