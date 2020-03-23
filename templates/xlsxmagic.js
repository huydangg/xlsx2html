const TIME_OUT_FOR_EACH_CHUNKS = 5000
const DELAY_TIME_FOR_EACH_LOOP = 1000
const BASE_URL = 'file:///media/huydang/HuyDang/xlsxmagic/output'
const originFileName = ''
var indexCurrentSheet = 0
var tempSetTimeOut = null

google.charts.load('current', {'packages':['corechart']})



function loadAllChunksForPrint(){
  countJsonFile++
  for(indexCurrentChunk; indexCurrentChunk <= chunkSize; indexCurrentChunk++){
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
}

function loadChunks(indexCurrentSheet, indexCurrentChunk, startTime) {
  var indexCurrentChunk = indexCurrentChunk;
  var startTime = startTime
  var isFailed = false
  var isDone = false
  var htmlFileName = "chunk_" + indexCurrentSheet + "_" + indexCurrentChunk
  var URL_HTML_CHUNK = BASE_URL + originFileName + "/chunks/" + htmlFileName  + '.html'

  readTextFile(URL_HTML_CHUNK, function(data){
    if(typeof data != 'undefinded') {
      startTime = new Date().getTime()
      if (indexCurrentChunk === 0) {


      } else {
	
      }
      indexCurrentChunk++
      loadChunks(indexCurrentSheet, indexCurrentChunk, startTime)
    } else {
      var endTime = new Date().getTime()
      if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
        isFailed = true
      }
      else{
        tempSetTimeOut = setTimeout(function(){loadChunks(indexCurrentSheet, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
      }
    }
  })
  if (isFailed || isDone) {
    return;
  }
}

function Viewer() {
  var current_sheet_ele = document.getElementById(indexCurrentSheet + '')
  current_sheet_ele.style.display = 'inline'
  loadChunks(indexCurrentSheet, 0, new Date().getTime())
}

document.addEventListener('DOMContentLoaded', function() {
  Viewer()
  }, false);
