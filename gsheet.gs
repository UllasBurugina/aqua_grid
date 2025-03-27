function doGet(e) {
  return handleRequest(e);
}

function doPost(e) {
  return handleRequest(e);
}

function handleRequest(e) {
  var sheetUrl = "https://docs.google.com/spreadsheets/d/1vhY10HYRuAwI5L-pcSfyxDrtIWGaFql9ILKQM2JPr88/edit?usp=sharing";  // Put your sheet URL here
  var sheet = SpreadsheetApp.openByUrl(sheetUrl).getSheetByName("Sheet3");  // Change to your sheet name
  
  var action = e.parameter.action;

  if (action == "update") {
    var row = parseInt(e.parameter.row);
    var column = parseInt(e.parameter.column);
    var value = e.parameter.value;
    
    sheet.getRange(row, column).setValue(value);
    
    return ContentService.createTextOutput("Cell Updated");
  }
  
  return ContentService.createTextOutput("Invalid Action");
}