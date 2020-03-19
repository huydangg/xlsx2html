const TIME_OUT_FOR_EACH_CHUNKS = 5000

google.charts.load('current', {'packages':['corechart']})

function loadAllChunksForPrint(){
  countJsonFile++
  for(countJsonFile; countJsonFile <= chunkSize; countJsonFile++){
    loadChunks(indexCurrentSheet, new Date().getTime())
  }
}

function readTextFile(file, callback) {
  var rawFile = new XMLHttpRequest()
  rawFile.overrideMimeType("application/json")
  rawFile.open("GET", file, false)
  rawFile.onreadystatechange = function() {
    try {
      rawFile.send(null)
    } catch(err) {
      console.log("rawFile.send err " + err)
      return "";
    }
}

function loadChunks(){
  var jsonFileName = "json__" + indexCurrentSheet + "__" + countJsonFile
  var URL_JSON = BASE_URL + originFileName + "/json/" + jsonFileName + "?access_token=" + accessToken
  readTextFile(URL_JSON, function(text){
    var data = JSON.parse(text)
    if(typeof data != 'undefinded') {
    } else {
    }
  })
}
loadChunks()
