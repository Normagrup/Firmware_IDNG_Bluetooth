var socket = new WebSocket("ws://" + window.location.hostname + ":4322");
var addressClicked = 0;
var nodesScanned = 0;
var nodesAdded = 0;

socket.onopen = function(event) { console.log('WebSocket connection established.'); };

socket.onerror = function(error) { console.error('WebSocket error:', error); };

socket.onmessage = function(event) {
    processReceivedData(event.data);
}

function processLoginInfo(value) 
{
    var signErrorLabel = document.getElementById('signError');

    console.log(value);

    if (value == 1) { 
        loadAdminMenu();
        signErrorLabel.style.visibility = "hidden";
    }
    else if (value == 2) {
        loadFactoryMenu();
        signErrorLabel.style.visibility = "hidden";
    }
    else {
        signErrorLabel.style.color = "#C30101";
        signErrorLabel.innerHTML = "<b>Username or password is wrong!</b>";
        signErrorLabel.style.visibility = "visible";
    }
}

function processInterfacesInfo(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var parts = value.split(' ');
    var ip = parts[0];
    var submask = parts[1];
    var gateway = parts[2];
    var buildingName = parts[3];
    var lineName = parts[4];

    iframeDocument.querySelector('[name="ipValue"]').value = ip;
    iframeDocument.querySelector('[name="submaskValue"]').value = submask;
    iframeDocument.querySelector('[name="gatewayValue"]').value = gateway;
    iframeDocument.querySelector('[name="buildingName"]').value = buildingName;
    iframeDocument.querySelector('[name="lineName"]').value = lineName;
}

function processIPConfigInfo(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var ipConfigLabel = iframeDocument.getElementById('ipConfigLabel');

    if (value == true) { 
        ipConfigLabel.style.color = "#4682b4";
        ipConfigLabel.innerHTML = "<b>Reboot the device!</b>";
        ipConfigLabel.style.visibility = "visible";
    } 
    else {
        ipConfigLabel.style.color = "#C30101";
        ipConfigLabel.innerHTML = "<b>Password is wrong!</b>";
        ipConfigLabel.style.visibility = "visible";
    }
}

function processDateTimeInfo(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var parts = value.split(' ');
    var dateParts = parts[0].split('-');
    var timeParts = parts[1].split(':');

    var day = dateParts[2];
    var month = dateParts[1];
    var year = dateParts[0];

    var hour = timeParts[0];
    var minutes = timeParts[1];

    var dateString = `${year}-${month}-${day}`;
    var timeString = `${hour}:${minutes}`;

    if (iframeDocument.location.href.includes('s_time.html')) {
        var datePicker = iframeDocument.getElementById('datePicker');
        var timePicker = iframeDocument.getElementById('timePicker');

        datePicker.value = dateString;
        timePicker.value = timeString;
    }
    else if (iframeDocument.location.href.includes('s_tests.html')) {
        var durationDatePicker = iframeDocument.getElementById('durationDatePicker');
        var functionTimePicker = iframeDocument.getElementById('functionTimePicker');
        var durationTimePicker = iframeDocument.getElementById('durationTimePicker');

        durationDatePicker.value = dateString;
        functionTimePicker.value = timeString;
        durationTimePicker.value = timeString;
    }
}

function addDeviceToScannedList(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var devices = scannedDevicesList.getElementsByTagName('li');
    
    for (var i = 0; i < devices.length; i++) {
        if (devices[i].textContent === value) { return; }
    }

    var newScanned = document.createElement('li');
    newScanned.textContent = value;
    newScanned.setAttribute('class', 'deviceScanned');
    newScanned.setAttribute('onclick', 'parent.selectDevice(this)');
    scannedDevicesList.appendChild(newScanned);
    nodesScanned++;

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;
}

function confirmStartCommission(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupHeader = popup.querySelector('h2');
    popupHeader.textContent = "Automatic commission in progress...";

    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = "0 / 0";

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
}

function startAddingDevices(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var devices = scannedDevicesList.getElementsByTagName('li');

    if (devices.length > 0) {
        var firstDevice = devices[0];
        var textDeviceSelected = firstDevice.textContent.trim();
        sendData("SET_START_ACTION", textDeviceSelected);
    }
    else { sendData("SET_NEW_COMMISSION_ITERATION", ""); }
}

function confirmAddingDevice(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupHeader = popup.querySelector('h2');
    popupHeader.textContent = "Adding device to the network...";

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
}

function addDeviceToNetworkList(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var devices = scannedDevicesList.getElementsByTagName('li');
    var firstDevice = devices[0];
    if (firstDevice) { firstDevice.remove(); }
    
    var networkNodesList = iframeDocument.getElementById('networkNodesList');
    var newNode = iframeDocument.createElement('li');
    newNode.textContent = "Node " + value;
    newNode.setAttribute('class', 'deviceNetwork');
    newNode.setAttribute('onclick', 'parent.selectDevice(this)');
    networkNodesList.appendChild(newNode);
    nodesAdded++;

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;
}

function processDeviceError(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var devices = scannedDevicesList.getElementsByTagName('li');
    var firstDevice = devices[0];
    firstDevice.remove();
    nodesScanned--;

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;

    // setTimeout(function() {
    //     if (devices.length > 0) {
    //         var firstDevice = devices[0];
    //         var textDeviceSelected = firstDevice.textContent.trim();
    //         sendData("SET_START_ACTION", textDeviceSelected);
    //     }
    //     else {
    //         sendData("SET_NEW_COMMISSION_ITERATION", "");
    //     }
    // }, 5000);
}

function processNodeInfo(value) 
{
    var nodeInfoArray = value.split('.');
    var controlGearStatus = nodeInfoArray[0];
    var emergencyMode = nodeInfoArray[1];
    var emergencyFailureStatus = nodeInfoArray[2];
    var actualLvl = nodeInfoArray[3];
    var communicationFailure = nodeInfoArray[4];
    var deviceType = nodeInfoArray[5];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var autonomyIcon = iframeDocument.getElementById('autonomyIcon');
    var batteryIcon = iframeDocument.getElementById('batteryIcon');
    var lampIcon = iframeDocument.getElementById('lampIcon');
    var comIcon = iframeDocument.getElementById('comIcon');
    var lvlIcon = iframeDocument.getElementById('lvlIcon');
    var emergencyIcon = iframeDocument.getElementById('emergencyIcon');
    var deviceTypeIcon = iframeDocument.getElementById('deviceTypeIcon');

    if ((controlGearStatus >> 1) & 1) { lampIcon.style.backgroundImage = "url('images/lampIconOnFail.png')"; }
    else { lampIcon.style.backgroundImage = "url('images/lampIcon.png')"; }

    if ((emergencyMode >> 2) & 1) { 
        emergencyIcon.style.backgroundImage = "url('images/emergencyIconOnFail.png')";
        emergencyIcon.style.backgroundColor = "#fdfab2";
    }
    else { 
        emergencyIcon.style.backgroundImage = "url('images/emergencyIcon.png')";
        emergencyIcon.style.backgroundColor = "#fff";
    }

    if ((emergencyFailureStatus >> 1) & 1) { autonomyIcon.style.backgroundImage = "url('images/autonomyIconOnFail.png')"; }
    else { autonomyIcon.style.backgroundImage = "url('images/autonomyIcon.png')"; }

    if ((emergencyFailureStatus >> 2) & 1) { batteryIcon.style.backgroundImage = "url('images/batteryIconOnFail.png')"; }
    else { batteryIcon.style.backgroundImage = "url('images/batteryIcon.png')"; }

    actualLvl = actualLvl / 254 * 100;
    if (actualLvl > 100) { actualLvl = 100; }
    lvlIcon.innerHTML = "<b>" + actualLvl + "</b>";
    lvlIcon.style.background = "linear-gradient(to top, #bcf4f7 " + actualLvl + "%, #fff " + actualLvl + "%)";

    if (communicationFailure != 0) { comIcon.style.backgroundImage = "url('images/comIconOnFail.png')"; }
    else { comIcon.style.backgroundImage = "url('images/comIcon.png')"; }

    if (deviceType == "1") { deviceTypeIcon.src = "images/emergencyLightIcon.png"; }
    else if (deviceType == "6") { deviceTypeIcon.src = "images/normalLightIcon.png"; }
}

function processEndNodeConfiguration(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var devices = scannedDevicesList.getElementsByTagName('li');

    setTimeout(function() {
        if (devices.length > 0) {
            var firstDevice = devices[0];
            var textDeviceSelected = firstDevice.textContent.trim();
            sendData("SET_START_ACTION", textDeviceSelected);
        }
        else {
            sendData("SET_NEW_COMMISSION_ITERATION", "");
        }
    }, 5000);
}

function processEndAutoCommission(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";
}

function processFactoryIDWrote(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var loader1 = iframeDocument.getElementById('loader1');
    loader1.classList.add('complete');
}

function processDaliTested(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var loader2 = iframeDocument.getElementById('loader2');
    loader2.classList.add('complete');
}

function processRecordedDevice(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var loader3 = iframeDocument.getElementById('loader3');
    loader3.classList.add('complete');

    var labelContainer = iframeDocument.getElementById('labelContainer');
    var settingsContainer = iframeDocument.getElementById('settingsContainer');
    var confirmationContainer = iframeDocument.getElementById('confirmationContainer');
    var codeContainer = iframeDocument.getElementById('codeContainer');

    setTimeout(function() {
        settingsContainer.style.display = 'none';
        confirmationContainer.style.display = 'flex';
    }, 3800);

    setTimeout(function() {
        labelContainer.style.display = 'flex';
        codeContainer.style.display = 'none';
    }, 7300);
}

function processReceivedData(data) 
{
    var dataArray = data.split('@');
    var type = dataArray[0];
    var value = dataArray[1];
    
    if (type == 'LOG_IN_INFO') { processLoginInfo(value); }
    else if (type == 'INTERFACES_INFO') { processInterfacesInfo(value); }
    else if (type == 'IPCONFIG_INFO') { processIPConfigInfo(value); }
    else if (type == 'DATE_TIME_INFO') { processDateTimeInfo(value); }
    else if (type == 'SCANNED_DEVICE') { addDeviceToScannedList(value); }
    else if (type == 'CONFIRM_START_COMMISSION') { confirmStartCommission(value); }
    else if (type == 'START_ADDING_DEVICES') { startAddingDevices(value); }
    else if (type == 'CONFIRM_ADDING_DEVICE') { confirmAddingDevice(value); }
    else if (type == 'ADDED_DEVICE') { addDeviceToNetworkList(value); }
    else if (type == 'DEVICE_ERROR') { processDeviceError(value); }
    else if (type == 'NODE_INFO') { processNodeInfo(value); }
    else if (type == 'END_NODE_CONFIG') { processEndNodeConfiguration(value); }
    else if (type == 'END_AUTO_COMMISSION') { processEndAutoCommission(value); }
    else if (type == 'FACTORY_ID_WROTE') { processFactoryIDWrote(value); }
    else if (type == 'DALI_TESTED') { processDaliTested(value); }
    else if (type == 'RECORDED_DEVICE') { processRecordedDevice(value); }
}

function sendData(type, value) 
{
    if (socket.readyState === WebSocket.OPEN) {
        var data = type + "@" + value;
        socket.send(data);
        console.log('Data sent.');
    } 
    else { console.error('WebSocket connection not open.'); }
}

function sendLogin() 
{
    var signErrorLabel = document.getElementById('signError');
    var username = document.getElementById('usernameInput').value;
    var password = document.getElementById('passwordInput').value;
    
    if (!username || !password) {
        signErrorLabel.style.color = "#C30101";
        signErrorLabel.innerHTML = "<b>Username or password is empty!</b>";
        signErrorLabel.style.visibility = "visible";
    }
    else {
        var message = username + ' ' + password;
        sendData("SET_LOG_IN", message);
    }
}

function sendIPConfig() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var ip = iframeDocument.querySelector('[name="ipValue"]').value;
    var submask = iframeDocument.querySelector('[name="submaskValue"]').value;
    var gateway = iframeDocument.querySelector('[name="gatewayValue"]').value;
    var buildingName = iframeDocument.querySelector('[name="buildingName"]').value;
    var lineName = iframeDocument.querySelector('[name="lineName"]').value;
    var password = iframeDocument.querySelector('[name="adminPass"]').value;

    if (!ip || !submask || !gateway || !buildingName || !lineName || !password) {
        var ipConfigLabel = iframeDocument.getElementById('ipConfigLabel');
        ipConfigLabel.style.color = "#C30101";
        ipConfigLabel.innerHTML = "<b>Some field is empty!</b>";
        ipConfigLabel.style.visibility = "visible";
    }
    else {
        var message = ip + ' ' + submask + ' ' + gateway + ' ' + buildingName + ' ' + lineName + ' ' + password;
        sendData("SET_IP_CONFIG", message);
    }
}

function sendDateTime() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var date = iframeDocument.getElementById('datePicker').value;
    var time = iframeDocument.getElementById('timePicker').value;

    var message = date + ' ' + time;
    sendData("SET_DATE_TIME", message);
}

function rebootDevice()
{
    var isTrue = confirm("You are going to reboot the IDNG-Blue! Are you sure?")
    if (isTrue) {
        sendData("SET_REBOOT_DEVICE", " ");
        logoutApp();
        window.location.href = "http://" + window.location.hostname;
    }
}

function getScannedDevices() 
{
    sendData("SET_SCANNED_DEVICES", "");
}

function startCommission() 
{
    sendData("SET_START_ACTION", "0");
}

function addDevice() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var networkErrorLabel = iframeDocument.getElementById('networkError');
    sendData("SET_START_ACTION", "FF");

    // var selectedDevice = iframeDocument.querySelector('#scannedDevicesList li.selectedDevice');
    // if (selectedDevice) {
    //     networkErrorLabel.style.visibility = "hidden";

    //     var textDeviceSelected = selectedDevice.textContent.trim();
    //     sendData("SET_START_ACTION", textDeviceSelected);
    // }
    // else {
    //     networkErrorLabel.style.color = "#C30101";
    //     networkErrorLabel.innerHTML = "<b> Select a device from scanned devices! </b>";
    //     networkErrorLabel.style.visibility = "visible";
    // }
}

function delDevice() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var networkErrorLabel = iframeDocument.getElementById('networkError');
    sendData("SET_DELETE_DEVICE", "");
    // var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
    // if (selectedNode) {
    //     networkErrorLabel.style.visibility = "hidden";

    //     var textNodeSelected = selectedNode.textContent.trim();
    //     sendData("SET_DELETE_DEVICE", textNodeSelected);
    //     selectedNode.remove();
    // }
    // else {
    //     networkErrorLabel.style.color = "#C30101";
    //     networkErrorLabel.innerHTML = "<b> Select a node from network nodes! </b>";
    //     networkErrorLabel.style.visibility = "visible";
    // }
}

function addToGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var groupErrorLabel = iframeDocument.getElementById('groupError');

    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if (selectedNode && groupSelected != '-') {
        groupErrorLabel.style.visibility = "hidden";
        var textNodeSelected = selectedNode.textContent.trim();
        var message = textNodeSelected + ' ' + groupSelected;
        sendData("SET_ADD_GROUP", message);
    }
    else {
        groupErrorLabel.style.color = "#C30101";
        groupErrorLabel.innerHTML = "<b> Pick a group and select a node from network nodes! </b>";
        groupErrorLabel.style.visibility = "visible";
    }
}

function delFromGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var groupErrorLabel = iframeDocument.getElementById('groupError');

    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if (selectedNode && groupSelected != '-') {
        groupErrorLabel.style.visibility = "hidden";
        var textNodeSelected = selectedNode.textContent.trim();
        var message = textNodeSelected + ' ' + groupSelected;
        sendData("SET_DEL_GROUP", message);
    }
    else {
        groupErrorLabel.style.color = "#C30101";
        groupErrorLabel.innerHTML = "<b> Pick a group and select a node from network nodes! </b>";
        groupErrorLabel.style.visibility = "visible";
    }
}

function maxButton() 
{
    sendData("SET_MAX", addressClicked);
}

function offButton() 
{
    sendData("SET_OFF", addressClicked);
}

function minButton() 
{
    sendData("SET_MIN", addressClicked);
}

function resetButton() 
{
    sendData("SET_RESET", addressClicked);
}

function sliderInput() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var sliderValue = iframeDocument.getElementById("lvlSlider").value;
    var message = addressClicked + ' ' + sliderValue;
    sendData("SET_ACTUAL_LVL", message);
}

function identifyButton() 
{
    sendData("SET_IDENTIFY", addressClicked);
}

function facSettingsButton() 
{
    sendData("SET_FACTORY_SETTINGS", addressClicked);
}

function rebootButton() 
{
    sendData("SET_REBOOT", addressClicked);
}

function funcTestButton()
{
    sendData("SET_FUNCTION_TEST", addressClicked);
}

function durTestButton()
{
    sendData("SET_DURATION_TEST", addressClicked);
}

function stopButton()
{
    sendData("SET_STOP", addressClicked);
}

function setTest()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var toggleFunction = iframeDocument.getElementById('toggleFunction');
    var toggleDuration = iframeDocument.getElementById('toggleDuration');

    var monday = iframeDocument.getElementById('monday');
    var tuesday = iframeDocument.getElementById('tuesday');
    var wednesday = iframeDocument.getElementById('wednesday');
    var thursday = iframeDocument.getElementById('thursday');
    var friday = iframeDocument.getElementById('friday');
    var saturday = iframeDocument.getElementById('saturday');
    var sunday = iframeDocument.getElementById('sunday');

    var testErrorLabel = iframeDocument.getElementById('testError');
    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;
    var periodicityList = iframeDocument.getElementById('periodicityList');
    var periodicitySelected = periodicityList.options[periodicityList.selectedIndex].value;

    if (groupSelected != '-') {
        testErrorLabel.style.visibility = "hidden";

        var message = groupSelected + ' ';
        if (toggleFunction.checked) {
            var functionTime = iframeDocument.getElementById('functionTimePicker').value;

            if (monday.checked || tuesday.checked || wednesday.checked || thursday.checked || friday.checked || saturday.checked || sunday.checked) {
                testErrorLabel.style.visibility = "hidden";

                if (monday.checked) { message += 'Mon-'; }
                if (tuesday.checked) { message += 'Tue-'; }
                if (wednesday.checked) { message += 'Wed-'; }
                if (thursday.checked) { message += 'Thu-'; }
                if (friday.checked) { message += 'Fri-'; }
                if (saturday.checked) { message += 'Sat-'; }
                if (sunday.checked) { message += 'Sun-'; }

                message += ' ' + functionTime + ' ';
            }
            else {
                testErrorLabel.style.color = "#C30101";
                testErrorLabel.innerHTML = "<b> Select one or more days! </b>";
                testErrorLabel.style.visibility = "visible";

                return;
            }
        }
        if (toggleDuration.checked) {
            var durationDate = iframeDocument.getElementById('durationDatePicker').value;
            var durationTime = iframeDocument.getElementById('durationTimePicker').value;

            if (periodicitySelected != '-') {
                message += periodicitySelected + ' ' + durationDate + ' ' + durationTime;
            }
            else {
                testErrorLabel.style.color = "#C30101";
                testErrorLabel.innerHTML = "<b> Select periodicity! </b>";
                testErrorLabel.style.visibility = "visible";

                return;
            }
        }

        testErrorLabel.style.color = "#4682b4";
        testErrorLabel.innerHTML = "<b> Test changed! </b>";
        testErrorLabel.style.visibility = "visible";
        sendData("SET_TEST", message);
    }
    else {
        testErrorLabel.style.color = "#C30101";
        testErrorLabel.innerHTML = "<b> Pick a group! </b>";
        testErrorLabel.style.visibility = "visible";
    }
}

function sendFile()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var fileInput = iframeDocument.getElementById('fileInput');
    var file = fileInput.files[0];
    const reader = new FileReader();

    reader.onload = function (event) {
        const fileContent = event.target.result;
        sendData("SET_UPDATE_FILE", fileContent);
        alert("Device will reboot now. Reload Page!");
    };

    reader.readAsArrayBuffer(file);
}

function getLogs()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var reportList = iframeDocument.getElementById('reportList');
    var reportListSelected = reportList.options[reportList.selectedIndex].value;
    var initialDatePicker = iframeDocument.getElementById('initialDatePicker');
    var finalDatePicker = iframeDocument.getElementById('finalDatePicker');
    var logErrorLabel = iframeDocument.getElementById('logError');

    var initialDate = new Date(initialDatePicker.value);
    var finalDate = new Date(finalDatePicker.value);

    if (reportListSelected != '-' && initialDatePicker.value && finalDatePicker.value) {
        if (initialDate <= finalDate) {
            var message = reportListSelected + ' ' + initialDatePicker.value + ' ' + finalDatePicker.value

            sendData("GET_LOGS", message);

            logErrorLabel.style.color = "#4682b4";
            logErrorLabel.innerHTML = "<b> Getting logs...! </b>";
            logErrorLabel.style.visibility = "visible";
        }
        else {
            logErrorLabel.style.color = "#C30101";
            logErrorLabel.innerHTML = "<b> Initial date is later than final date! </b>";
            logErrorLabel.style.visibility = "visible";
        }
    }
    else {
        logErrorLabel.style.color = "#C30101";
        logErrorLabel.innerHTML = "<b> Pick date and report type! </b>";
        logErrorLabel.style.visibility = "visible";
    }
}

function openNodeControl(buttonText)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupText = popup.querySelector('h3');
    popupText.textContent = "A" + buttonText;
    addressClicked = buttonText;
    
    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    sendData("GET_NODE_INFO", addressClicked);
}

function closeNodeControl(buttonText)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');
    
    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";

    sendData("SET_CLOSE_CONTROL", "");
}

function codeReaderChanged()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var codeReader = iframeDocument.getElementById('codeReader');
    var labelContainer = iframeDocument.getElementById('labelContainer');
    var codeContainer = iframeDocument.getElementById('codeContainer');
    var settingsContainer = iframeDocument.getElementById('settingsContainer');
    var confirmationContainer = iframeDocument.getElementById('confirmationContainer');

    var loader1 = iframeDocument.getElementById('loader1');
    var loader2 = iframeDocument.getElementById('loader2');
    var loader3 = iframeDocument.getElementById('loader3');
    loader1.classList.remove('complete');
    loader2.classList.remove('complete');
    loader3.classList.remove('complete');

    settingsContainer.style.display = 'flex';
    confirmationContainer.style.display = 'none';

    var labelParts = codeReader.value.split('ñ');
    var labelCodeParts = labelParts[1];
    var codeParts = labelCodeParts.split('.');
    var labelCode1 = iframeDocument.getElementById('labelCode1');
    var labelCode2 = iframeDocument.getElementById('labelCode2');
    var labelCode3 = iframeDocument.getElementById('labelCode3');
    var labelCode4 = iframeDocument.getElementById('labelCode4');

    labelCode1.innerHTML = codeParts[0];
    labelCode2.innerHTML = codeParts[1];
    labelCode3.innerHTML = codeParts[2];
    labelCode4.innerHTML = codeParts[3];

    labelContainer.style.display = 'none';
    codeContainer.style.display = 'flex';

    sendData("SET_READ_ID_CODE", codeReader.value);

    codeReader.value = '';
}