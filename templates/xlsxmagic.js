const TIME_OUT_FOR_EACH_CHUNKS = 2000
const DELAY_TIME_FOR_EACH_LOOP = 500
var indexCurrentSheet = 0
var tempSetTimeOut = null
var currentSheetEle = null
var currentTableChunkEle = null
var currentTheadChunkEle = null
var currentTbodyChunkEle = null
google['charts'].load('current', {'packages':['corechart']})
const zip = (arr, ...arrs) => {
  return arr.map((val, i) => arrs.reduce((a, arr) => [...a, arr[i]], [val]));
}
function readTextFile(file, file_type, callback, callbackfail) {
  var rawFile = new XMLHttpRequest()
  if (file_type === "json") {
    rawFile.overrideMimeType("application/json")
  } else {
    rawFile.overrideMimeType("text/plain")
  }
  rawFile.open("GET", file, false)
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
function loadMergedCells(indexCurrentSheet, startTime) {
  var isFailed = false
  var isDone = false
  var mergedCellsFileName = "chunk_" + indexCurrentSheet + "_mc"
  var currentDivChunkEle = document.getElementById(mergedCellsFileName)
  if (currentDivChunkEle === null) {
    return
  }
  var URL_JSON_CHUNK = currentDivChunkEle.getAttribute('data-chunk-url')
  var _data = null
  readTextFile(URL_JSON_CHUNK, 'json', function(data){
    if(data !== void 0) {
      startTime = new Date().getTime()
      _data = JSON.parse(data)
      isDone = true
    } else {
      var endTime = new Date().getTime()
      if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
        isFailed = true
      }
      else {
        tempSetTimeOut = setTimeout(function(){loadMergedCells(indexCurrentSheet, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
      }
    }
  }, function(){
       // handle fail
       var endTime = new Date().getTime()
       if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
         isFailed = true
       }
       else{
         tempSetTimeOut = setTimeout(function(){loadMergedCells(indexCurrentSheet, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
       }
     })
  if (isDone) {
    return _data
  }
  if (isFailed) {
    return
  }
}
function loadChunks(indexCurrentSheet, indexCurrentChunk, startTime) {
  var isFailed = false
  var isDone = false
  var htmlFileName = "chunk_" + indexCurrentSheet + "_" + indexCurrentChunk
  var currentDivChunkEle = document.getElementById(htmlFileName)
  if (currentDivChunkEle === null) {
    return
  }
  var URL_HTML_CHUNK = currentDivChunkEle.getAttribute('data-chunk-url')
  readTextFile(URL_HTML_CHUNK, 'html', function(data){
    if(data !== void 0) {
      startTime = new Date().getTime()
      if (indexCurrentChunk === 0) {
	currentTheadChunkEle.innerHTML = data
      } else if (indexCurrentChunk == 1) {
	currentTbodyChunkEle.innerHTML = data
      }
      loadImg(indexCurrentSheet)
      google['charts']['setOnLoadCallback'](function(){loadChart(indexCurrentSheet, 0, new Date().getTime())})
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
         tempSetTimeOut = setTimeout(function(){loadChunks(indexCurrentSheet, indexCurrentChunk, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
       }
     })
  if (isFailed || isDone) {
    return;
  }
}
function applyMergedCells(mergedCellsData) {
  for (var key in mergedCellsData) {
    var mergedCellEle = document.getElementById(indexCurrentSheet + '_' + key)
    mergedCellEle.colSpan = mergedCellsData[key]['colspan']
    var countColSpan = mergedCellsData[key]['colspan']
    var rowEle = mergedCellEle.parentNode
    while (countColSpan > 1) {
      var removedMergeCellEle = mergedCellEle.nextElementSibling
      rowEle.removeChild(removedMergeCellEle)
      countColSpan--
    }
    var splitKey = key.match(/([A-Z]+)([0-9]+)/)
    var columnName = splitKey[1]
    var countRowSpan = mergedCellsData[key]['rowspan']
    while (countRowSpan > 1) {
      countColSpan = mergedCellsData[key]['colspan']
      rowEle = rowEle.nextElementSibling
      var mergedCellEleNextRow = document.getElementById(indexCurrentSheet + '_' + columnName + rowEle.id) 
      while (countColSpan > 1) {
        var removedMergeCellEle = mergedCellEleNextRow.nextElementSibling
        if (!removedMergeCellEle) {
	  break
        }
        rowEle.removeChild(removedMergeCellEle)
	countColSpan--
      }
      rowEle.removeChild(mergedCellEleNextRow)
      countRowSpan--
    }
    mergedCellEle.rowSpan = mergedCellsData[key]['rowspan']
  }
}
function getOffset(el) {
  const rect = el.getBoundingClientRect()
  return {
    left: rect.left + currentSheetEle.scrollLeft,
    top: rect.top + currentSheetEle.scrollTop
  }
}
function loadImg(indexCurrentSheet) {
  var indexImg = 0
  while (1) {
    var divImgMetaData = document.getElementById("chunk_" + indexCurrentSheet + "_" + indexImg + '_img');
    if (!divImgMetaData) {
      return;
    }
    var row = divImgMetaData.getAttribute('data-from-row')
    var col = divImgMetaData.getAttribute('data-from-col')
    var cell = document.getElementById(indexCurrentSheet + '_' + (col + row))
    if (!cell) {
      return
    }
    var colOff = divImgMetaData.getAttribute('data-from-coloff')
    var rowOff = divImgMetaData.getAttribute('data-from-rowoff')
    var height = divImgMetaData.getAttribute('data-height')
    var width = divImgMetaData.getAttribute('data-width')

    var reactCell = getOffset(cell)
    var imgEle = document.createElement('img')
    imgEle.src = divImgMetaData.getAttribute('data-img-url')
    imgEle.style.top = reactCell.top + parseInt(rowOff, 10) + 'px'
    imgEle.style.left = reactCell.left + parseInt(colOff, 10) + 'px'
    imgEle.style.position = 'absolute'
    imgEle.heigth = height
    imgEle.width = width
    currentSheetEle.appendChild(imgEle)
    indexImg++
  }
}
function loadChart(indexCurrentSheet, indexChart, startTime) {
  var isFailed = false
  var isDone = false
  var divChartMetaData = document.getElementById("chunk_" + indexCurrentSheet + "_" + indexChart + '_chart');
  if (!divChartMetaData) {
    return;
  }
  var URL_CHART_CHUNK = divChartMetaData.getAttribute('data-chart-url')
  readTextFile(URL_CHART_CHUNK , 'json', function(data){
    if(data !== void 0) {
      data = JSON.parse(data)
      startTime = new Date().getTime()
      var row = divChartMetaData.getAttribute('data-from-row')
      var col = divChartMetaData.getAttribute('data-from-col')
      var height = divChartMetaData.getAttribute('data-height')
      var width = divChartMetaData.getAttribute('data-width')
      var cell = document.getElementById(indexCurrentSheet + '_' + (col + row))
      if (!cell) {
        return
      }
      var colOff = divChartMetaData.getAttribute('data-from-coloff')
      var rowOff = divChartMetaData.getAttribute('data-from-rowoff')
      var reactCell = getOffset(cell)
      var divChart = document.createElement('div')
      divChart.id = indexCurrentSheet + '-' + indexChart
      divChart.style.position = 'absolute'
      divChart.style.top = reactCell.top + parseInt(rowOff, 10) + 'px'
      divChart.style.left = reactCell.left + parseInt(colOff, 10) + 'px'
      divChart.style.width = width
      divChart.style.heigtht = height
      currentSheetEle.appendChild(divChart)

      var data_table = [];
      var options = {
        title: data['title'] ? data['title']['text'] : ""
      }
      var chart;
      for (var i_chart = 0; i_chart < data['charts'].length; i_chart++) {
        if (data['charts'][i_chart]['type'] === 'barChart') {
	  if (data['charts'][i_chart]['barDir'] === 'col') {
            chart = new google['visualization']['ColumnChart'](divChart)
	  } else if (data['charts'][i_chart]['barDir'] === 'bar') {
            chart = new google['visualization']['BarChart'](divChart)
	  }
	  var col_name = ['Row']
	  var sers = []
	  for (var i_ser = 0; i_ser < data['charts'][i_chart]['sers'].length; i_ser++) {
	    col_name.push(data['charts'][i_chart]['sers'][i_ser]['tx'])
	    sers.push(data['charts'][i_chart]['sers'][i_ser]['val'])
	  }
	  data_table.push(col_name)
	  data_table.push(...zip(data['charts'][i_chart]['sers'][0]['cat'], ...sers))
	} else if (data['charts'][i_chart]['type'] === 'bar3DChart') {
          options['is3D'] = true
	  if (data['charts'][i_chart]['barDir'] === 'col') {
            chart = new google['visualization']['ColumnChart'](divChart)
	  } else if (data['charts'][i_chart]['barDir'] === 'bar') {
            chart = new google['visualization']['BarChart'](divChart)
	  }
	}
      }
      data_table = google['visualization']['arrayToDataTable'](data_table)
      if (chart !== void 0)
        chart['draw'](data_table, options)
      indexChart++
      loadChart(indexCurrentSheet, indexChart, startTime)
    } else {
      var endTime = new Date().getTime()
      if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
        isFailed = true
      }
      else {
        tempSetTimeOut = setTimeout(function(){loadChart(indexCurrentSheet, indexChart, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
      }
    }
  }, function(){
       // handle fail
       var endTime = new Date().getTime()
       if (endTime - startTime  >= TIME_OUT_FOR_EACH_CHUNKS) {
         isFailed = true
       }
       else{
         tempSetTimeOut = setTimeout(function(){loadChart(indexCurrentSheet, indexChart, startTime)}, DELAY_TIME_FOR_EACH_LOOP)
       }
  })

  if (isFailed || isDone) {
    return;
  }
}


function Viewer() {
  currentSheetEle = document.getElementById('sheet_' + indexCurrentSheet)
  currentSheetEle.style.removeProperty("display")
  if (!document.getElementById('tb_' + indexCurrentSheet)) {
    currentTableChunkEle = currentSheetEle.appendChild(document.createElement('table'))
    currentTableChunkEle.id = "tb_" + indexCurrentSheet
    currentTheadChunkEle = currentTableChunkEle.appendChild(document.createElement('thead'))
    currentTbodyChunkEle = currentTableChunkEle.appendChild(document.createElement('tbody'))
    var mergedCellsData = loadMergedCells(indexCurrentSheet, new Date().getTime())
    loadChunks(indexCurrentSheet, 0, new Date().getTime())
    if (mergedCellsData) {
      applyMergedCells(mergedCellsData)
    }
  }
}
function handleButtonClick(event) {
  var btnClicked = document.getElementById(event.srcElement.id)
  if ('btn_' + indexCurrentSheet == btnClicked.id) {
    return
  }
  currentSheetEle.style.display = 'none'
  var preBtn = document.getElementById('btn_' + indexCurrentSheet)
  preBtn.style.fontWeight = 'normal'
  indexCurrentSheet = btnClicked.id.split("btn_")[1]
  btnClicked.style.fontWeight = 'bold'
  Viewer()
  event.preventDefault();
}
window['handleButtonClick'] = handleButtonClick
document.addEventListener('DOMContentLoaded', function() {
  Viewer()
}, false);
