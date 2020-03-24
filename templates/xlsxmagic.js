const TIME_OUT_FOR_EACH_CHUNKS = 5000
const DELAY_TIME_FOR_EACH_LOOP = 1000
const originFileName = ''
var indexCurrentSheet = 0
var tempSetTimeOut = null
var currentSheetEle = null
var currentTableChunkEle = null
var currentTheadChunkEle = null
var currentTbodyChunkEle = null

google.charts.load('current', {'packages':['corechart']})



function loadAllChunksForPrint(){
  countJsonFile++
  for(indexCurrentChunk; indexCurrentChunk <= chunkSize; indexCurrentChunk++){
    loadChunks(indexCurrentSheet, new Date().getTime())
  }
}

function readTextFile(file, callback, callbackfail) {
  var rawFile = new XMLHttpRequest()
  rawFile.overrideMimeType("text/plain")
  rawFile.open("GET", file, false)
  console.log("AAAAAAAAAA")
  rawFile.onreadystatechange = function() {
    if (rawFile.readyState === 4) {
      callback(rawFile.responseText);
    }
    else{
      callbackfail()
    }
  }
  rawFile.onerror = function () {
    callbackfail()
  }
  rawFile.send(null)
}

function loadChunks(indexCurrentSheet, indexCurrentChunk, startTime) {
  var indexCurrentChunk = indexCurrentChunk;
  var startTime = startTime
  var isFailed = false
  var isDone = false
  var htmlFileName = "chunk_" + indexCurrentSheet + "_" + indexCurrentChunk
  console.log(htmlFileName)
  var currentDivChunkEle = document.getElementById(htmlFileName)
  if (currentDivChunkEle === null) {
    return
  }
  
  var URL_HTML_CHUNK = currentDivChunkEle.getAttribute('data-chunk-url')
  console.log(URL_HTML_CHUNK)

  readTextFile(URL_HTML_CHUNK, function(data){
    console.log(data)
    if(typeof data != 'undefinded') {
      startTime = new Date().getTime()
      if (indexCurrentChunk === 0) {
	currentTheadChunkEle.innerHTML = data
      } else if (indexCurrentChunk == 1) {
	console.log(currentTbodyChunkEle)
	currentTbodyChunkEle.innerHTML = data
      }
      indexCurrentChunk++
      loadChunks(indexCurrentSheet, indexCurrentChunk, startTime)
    } else {
      var endTime = new Date().getTime()
      if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
        isFailed = true
      }
      else {
        tempSetTimeOut = setTimeout(function(){loadChunks(indexCurrentSheet, indexCurrentChunk, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
      }
    }
  }, function(){
       // handle fail
       var endTime = new Date().getTime()
       if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
         isFailed = true
       }
       else{
         tempSetTimeOut = setTimeout(function(){loadTheFirstChunk(startTime)}, DELAY_TIME_FOR_EACH_LOOP)
       }
     })
  if (isFailed || isDone) {
    return;
  }
}

function Viewer() {
  currentSheetEle = document.getElementById('sheet_' + indexCurrentSheet)
  currentSheetEle.style.display = 'inline'
  currentTableChunkEle = currentSheetEle.appendChild(document.createElement('table'))
  currentTheadChunkEle = currentTableChunkEle.appendChild(document.createElement('thead'))
  currentTbodyChunkEle = currentTableChunkEle.appendChild(document.createElement('tbody'))
  loadChunks(indexCurrentSheet, 0, new Date().getTime())
}

document.addEventListener('DOMContentLoaded', function() {
  Viewer()
  }, false);
