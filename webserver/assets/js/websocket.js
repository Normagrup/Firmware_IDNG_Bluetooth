var socket = new WebSocket("ws://" + window.location.hostname + ":4322");
var addressClicked = 0;
var nodesScanned = 0;
var nodesAdded = 0;
var allGroups = []; // Stores groups globally
var groupsPerPage = 15;
var currentPage = 1;

const fixedGroups = [
    { name: "Lighting", address: "C000" },
    { name: "Emergency", address: "C001" },
    { name: "Even", address: "C002" },
    { name: "Odd", address: "C003" }
];

socket.onopen = function(event) { console.log('WebSocket connection established.'); };

socket.onerror = function(error) { console.error('WebSocket error:', error); };

socket.onmessage = function(event) {
    processReceivedData(event.data);
}

function processAlertCommission(value)
{
    alert(value);
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

    var labelCommissionNodes = iframeDocument.getElementById('labelCommissionNodes');
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

    var parts = value.split("_");
    var nodeNetAddress = parts[0];
    var serialNumber = parts[1];
    
    var networkNodesList = iframeDocument.getElementById('networkNodesList');
    var newNode = iframeDocument.createElement('li');
    newNode.textContent = "Node " + nodeNetAddress + " - [" + serialNumber + "]";
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
    if (!value || value.trim() === "" || value.split('.').length < 6) {
        console.warn("Invalid node info received.");
        return;
    }

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

    autonomyIcon.innerHTML = "";
    batteryIcon.innerHTML = "";
    lampIcon.innerHTML = "";
    comIcon.innerHTML = "";
    emergencyIcon.innerHTML = "";

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
    var actualLvlNum = parseFloat(actualLvl);
    var actualLvlFormatted = (actualLvlNum % 1 === 0) ? actualLvlNum.toString() : actualLvlNum.toFixed(2);
    lvlIcon.innerHTML = "<b>" + actualLvlFormatted + "</b>";
    lvlIcon.style.background = "linear-gradient(to top, #bcf4f7 " + actualLvlFormatted + "%, #fff " + actualLvlFormatted + "%)";

    if (communicationFailure != 0) { comIcon.style.backgroundImage = "url('images/comIconOnFail.png')"; }
    else { comIcon.style.backgroundImage = "url('images/comIcon.png')"; }

    if (deviceType == "1") { deviceTypeIcon.src = "images/emergencyLightIcon.png"; }
    else if (deviceType == "6") { deviceTypeIcon.src = "images/normalLightIcon.png"; }
    else { deviceTypeIcon.src = "images/defaultLightIcon.png"; }
}

function processGroupBasicInfo(value) {
    var parts = value.split("_");
    var groupAddress = parts[0];
    var groupName = parts[1];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    // Procesado si el mensaje se recibe en network.html: Se crea el botón de ese grupo
    var container = iframeDocument.getElementById('group-container');
    if(container)
    {
        var groupButton = iframeDocument.createElement('button');
        groupButton.textContent = groupName;
        groupButton.onclick = function() {
            openGroupControl(this);
        };
        groupButton.setAttribute('group-address', groupAddress);
        container.appendChild(groupButton);
    }

    // Procesado si el mensaje se recibe en s_wireless.html o s_tests.html: Se crea el elemento en el selector
    var groupSelector = iframeDocument.getElementById('groupList');
    if(groupSelector)
    {
        var group = iframeDocument.createElement('option');
        group.value = groupAddress;
        group.textContent = groupName;

        groupSelector.appendChild(group);
    }

    var groupSelector2 = iframeDocument.getElementById('groupList2');
    if(groupSelector2)
    {
        var group2 = iframeDocument.createElement('option');
        group2.value = groupAddress;
        group2.textContent = groupName;

        groupSelector2.appendChild(group2);
    }
}

function processGroupInfo(value) 
{
    var groupInfoArray = value.split('.');
    var lampFailures = groupInfoArray[0];
    var emergencyModeCount = groupInfoArray[1];
    var batFailures = groupInfoArray[2];
    var durFailures = groupInfoArray[3];
    var averageLvl = groupInfoArray[4];
    var comFailures = groupInfoArray[5];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var autonomyIcon = iframeDocument.getElementById('autonomyIcon');
    var batteryIcon = iframeDocument.getElementById('batteryIcon');
    var lampIcon = iframeDocument.getElementById('lampIcon');
    var comIcon = iframeDocument.getElementById('comIcon');
    var lvlIcon = iframeDocument.getElementById('lvlIcon');
    var emergencyIcon = iframeDocument.getElementById('emergencyIcon');

    autonomyIcon.innerHTML = "";
    batteryIcon.innerHTML = "";
    lampIcon.innerHTML = "";
    comIcon.innerHTML = "";
    emergencyIcon.innerHTML = "";

    if (lampFailures > 0) { 
        lampIcon.style.backgroundImage = "url('images/lampIconOnFail.png')";
        
        var lampFailuresCountSpan = iframeDocument.createElement("span");
        lampFailuresCountSpan.classList.add("fail-count-badge");
        lampFailuresCountSpan.textContent = lampFailures;
        lampIcon.appendChild(lampFailuresCountSpan);
    }
    else { lampIcon.style.backgroundImage = "url('images/lampIcon.png')"; }

    if (emergencyModeCount > 0) { 
        emergencyIcon.style.backgroundImage = "url('images/emergencyIconOnFail.png')";
        emergencyIcon.style.backgroundColor = "#fdfab2";

        var emergencyModeCountSpan = iframeDocument.createElement("span");
        emergencyModeCountSpan.classList.add("emergency-count-badge");
        emergencyModeCountSpan.textContent = emergencyModeCount;
        emergencyIcon.appendChild(emergencyModeCountSpan);
    }
    else { 
        emergencyIcon.style.backgroundImage = "url('images/emergencyIcon.png')";
        emergencyIcon.style.backgroundColor = "#fff";
    }

    if (durFailures > 0) { 
        autonomyIcon.style.backgroundImage = "url('images/autonomyIconOnFail.png')";

        var durFailuresCountSpan = iframeDocument.createElement("span");
        durFailuresCountSpan.classList.add("fail-count-badge");
        durFailuresCountSpan.textContent = durFailures;
        autonomyIcon.appendChild(durFailuresCountSpan);
    } 
    else { autonomyIcon.style.backgroundImage = "url('images/autonomyIcon.png')"; }

    if (batFailures > 0) { 
        batteryIcon.style.backgroundImage = "url('images/batteryIconOnFail.png')"; 

        var batFailuresCountSpan = iframeDocument.createElement("span");
        batFailuresCountSpan.classList.add("fail-count-badge");
        batFailuresCountSpan.textContent = batFailures;
        batteryIcon.appendChild(batFailuresCountSpan);
    }
    else { batteryIcon.style.backgroundImage = "url('images/batteryIcon.png')"; }

    averageLvl = averageLvl / 254 * 100;
    if (averageLvl > 100) { averageLvl = 100; }
    var averageLvlNum = parseFloat(averageLvl);
    var averageLvlFormatted = (averageLvlNum % 1 === 0) ? averageLvlNum.toString() : averageLvlNum.toFixed(2);
    lvlIcon.innerHTML = "<b>" + averageLvlFormatted + "</b>";
    lvlIcon.style.background = "linear-gradient(to top, #bcf4f7 " + averageLvlFormatted + "%, #fff " + averageLvlFormatted + "%)";

    if (comFailures != 0) { 
        comIcon.style.backgroundImage = "url('images/comIconOnFail.png')";

        var comFailuresCountSpan = iframeDocument.createElement("span");
        comFailuresCountSpan.classList.add("fail-count-badge");
        comFailuresCountSpan.textContent = comFailures;
        comIcon.appendChild(comFailuresCountSpan);
    }
    else { comIcon.style.backgroundImage = "url('images/comIcon.png')"; }
}

function processTestData(value) {
    var testArray = value.split('#');
    var functionalEnable = testArray[0];
    var durationEnable = testArray[1];
    var functionalDays = testArray[2];
    var functionalTime = testArray[3];
    var durationPeriodicity = testArray[4];
    var durationDate = testArray[5];
    var durationTime = testArray[6]

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var toggleFunction = iframeDocument.getElementById('toggleFunction');
    toggleFunction.checked = (functionalEnable === "1") ? true : false;

    var toggleDuration = iframeDocument.getElementById('toggleDuration');
    toggleDuration.checked = (durationEnable === "1") ? true : false;

    var monday = iframeDocument.getElementById('monday'); monday.checked = false;
    var tuesday = iframeDocument.getElementById('tuesday'); tuesday.checked = false;
    var wednesday = iframeDocument.getElementById('wednesday'); wednesday.checked = false;
    var thursday = iframeDocument.getElementById('thursday'); thursday.checked = false;
    var friday = iframeDocument.getElementById('friday'); friday.checked = false;
    var saturday = iframeDocument.getElementById('saturday'); saturday.checked = false;
    var sunday = iframeDocument.getElementById('sunday'); sunday.checked = false;

    var days = functionalDays.split(' ');
    for (var day of days) {
        if(day === "Mon") { monday.checked = true; }
        else if(day === "Tue") { tuesday.checked = true; }
        else if(day === "Wed") { wednesday.checked = true; }
        else if(day === "Thu") { thursday.checked = true; }
        else if(day === "Fri") { friday.checked = true; }
        else if(day === "Sat") { saturday.checked = true; }
        else if(day === "Sun") { sunday.checked = true; }
    }

    var functionTime = iframeDocument.getElementById('functionTimePicker');
    functionTime.value = functionalTime;

    var periodicityList = iframeDocument.getElementById('periodicityList');
    if(durationPeriodicity === "0") { periodicityList.selectedIndex = 0; }
    else if(durationPeriodicity === "1") { periodicityList.selectedIndex = 1; }
    else if(durationPeriodicity === "3") { periodicityList.selectedIndex = 2; }
    else if(durationPeriodicity === "6") { periodicityList.selectedIndex = 3; }
    else if(durationPeriodicity === "12") { periodicityList.selectedIndex = 4; }

    var durationDateElem = iframeDocument.getElementById('durationDatePicker');
    if(durationDate !== "0000-00-00") { durationDateElem.value = durationDate; }
    else { durationDateElem.value = "2000-01-01"}

    var durationTimeElem = iframeDocument.getElementById('durationTimePicker');
    durationTimeElem.value = durationTime;

    var testErrorLabel = iframeDocument.getElementById('testError');
    testErrorLabel.innerHTML = " ‎ ";
}

function processDevicesCounter(value) {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var devicesCounter = iframeDocument.getElementById('devicesCounter');
    if (devicesCounter) {devicesCounter.textContent = value;}

    var devicesCounterNet = iframeDocument.getElementById('totalDevices');
    if (devicesCounterNet) {devicesCounterNet.textContent = value;}
}

function processFailuresCounter(value) {
    var parts = value.split(".");
    var totalFail = parts[0];
    var lampFail = parts[1];
    var batFail = parts[2];
    var durFail = parts[3];
    var comFail = parts[4];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var failuresCounter = iframeDocument.getElementById('failuresCounter'); // counter for summery.html
    if (failuresCounter) {failuresCounter.textContent = totalFail;}

    var failuresCounterNet = iframeDocument.getElementById('totalFailures'); // counter for network.html
    if (failuresCounterNet) {failuresCounterNet.textContent = totalFail;}
    
    // updating device count and fail count for network.html
    var lamFailIcon = iframeDocument.getElementById('LamFailType');
    var batFailIcon = iframeDocument.getElementById('BatFailType');
    var durFailIcon = iframeDocument.getElementById('DurFailType');
    var comFailIcon = iframeDocument.getElementById('comFailType');

    if (lamFailIcon) {lamFailIcon.src = (lampFail > 0) ? "images/lampIconOnFail.png" : "images/lampIcon.png";}
    if (batFailIcon) {batFailIcon.src = (batFail > 0) ? "images/batteryIconOnFail.png" : "images/batteryIcon.png";}
    if (durFailIcon) {durFailIcon.src = (durFail > 0) ? "images/autonomyIconOnFail.png" : "images/autonomyIcon.png";}
    if (comFailIcon) {comFailIcon.src = (comFail > 0) ? "images/comIconOnFail.png" : "images/comIcon.png";}

    var lamFailCount = iframeDocument.getElementById('LamFailCount');
    var batFailCount = iframeDocument.getElementById('BatFailCount');
    var durFailCount = iframeDocument.getElementById('DurFailCount');
    var comFailCount = iframeDocument.getElementById('comFailCount');
    
    if (lamFailCount) {lamFailCount.textContent = lampFail;}
    if (batFailCount) {batFailCount.textContent = batFail;}
    if (durFailCount) {durFailCount.textContent = durFail;}
    if (comFailCount) {comFailCount.textContent = comFail;}
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

function processIsConfig(value)
{
    var parts = value.split("_");
    var device = parts[0];
    var sn = parts[1];
    var isConfig = parts[2];
    var hasFailures = parts[3];
    var onOffStatus = parts[4];
    var configured = (isConfig === "true");
    var failed = (hasFailures === "true");
    var onOff = (onOffStatus === "on");

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var button = iframeDocument.querySelector('button[data-device="' + device + '"]');
    if (button) {
        button.setAttribute('data-serial', sn);
        button.innerHTML = "<b>" + device + " </b> <br>" + sn;

        if(configured) {
            if(failed) {
                button.classList.remove("blue", "gray");
                button.classList.add("red");
            } else {
                button.classList.remove("red", "gray");
                button.classList.add("blue");
            }
            button.disabled = false;
            button.innerHTML += '<span class="status-indicator ' + (onOff ? 'on-state' : 'off-state') + '"></span>';
        } else {
            button.classList.remove("red", "blue"); 
            button.classList.add("gray");
            button.disabled = true;
        }
    }
}

function processPowerOnLevelGroup(value) {
    var parts = value.split("_");
    var groupAddress = parts[0];
    var groupName = parts[1];
    var powerOnValue = parts[2];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var powerOnGroupSelector = iframeDocument.getElementById('groupListPowerOn');
    var powerLevelText = (powerOnValue === "0") ? "Off" : (powerOnValue === "254") ? "Max" : "Last Value";

    fixedGroups.forEach(group => {
        if (!powerOnGroupSelector.querySelector(`option[value="${group.address}"]`)) {
            var opt = iframeDocument.createElement('option');
            opt.value = group.address;
            opt.textContent = group.name;
            powerOnGroupSelector.appendChild(opt);
        }
    });

    if (!powerOnGroupSelector.querySelector(`option[value="${groupAddress}"]`)) {
        var opt = iframeDocument.createElement('option');
        opt.value = groupAddress;
        opt.textContent = groupName;
        powerOnGroupSelector.appendChild(opt);
    }

    let updated = false;

    fixedGroups.forEach(group => {
        if(group.address === groupAddress){
            let target = allGroups.find(g => g.address === group.address);
            if(target){
                target.powerOnValue = powerLevelText;
                updated = true;
            }
        }
    });

    if (!updated) {
        let existing = allGroups.find(g => g.address === groupAddress);
        if (existing) {
            existing.powerOnValue = powerLevelText;
        } else {
            allGroups.push({ address: groupAddress, name: groupName, powerOnValue: powerLevelText });
        }
    }

    updatePaginationDropdown(iframeDocument);
    displayPowerOnGroups(iframeDocument, 1); // always reset to page 1
    iframeDocument.getElementById('paginationDropdown').value = "1";
}

function processReceivedData(data) 
{
    var dataArray = data.split('@');
    var type = dataArray[0];
    var value = dataArray[1];
    
    if(type == 'ALERT_COMMISSION') { processAlertCommission(value); }
    else if (type == 'LOG_IN_INFO') { processLoginInfo(value); }
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
    else if (type == 'GROUP_NAME_AND_ADDRESS') { processGroupBasicInfo(value); }
    else if (type == "GROUP_INFO") { processGroupInfo(value); }
    else if (type == "TEST_DATA") { processTestData(value); }
    else if (type == "DEVICES_COUNTER") { processDevicesCounter(value); }
    else if (type == "FAILURES_COUNTER") { processFailuresCounter(value); }
    else if (type == 'END_NODE_CONFIG') { processEndNodeConfiguration(value); }
    else if (type == 'END_AUTO_COMMISSION') { processEndAutoCommission(value); }
    else if (type == 'FACTORY_ID_WROTE') { processFactoryIDWrote(value); }
    else if (type == 'DALI_TESTED') { processDaliTested(value); }
    else if (type == 'RECORDED_DEVICE') { processRecordedDevice(value); }
    else if (type == 'IS_CONFIG') { processIsConfig(value); }
    else if (type == 'POWER_ON_LVL') { processPowerOnLevelGroup(value)}
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

// function delDevice() 
// {
//     var iframe = document.getElementById('mainframe');
//     var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
//     var networkErrorLabel = iframeDocument.getElementById('networkError');
//     sendData("SET_DELETE_DEVICE", "");
//     // var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
//     // if (selectedNode) {
//     //     networkErrorLabel.style.visibility = "hidden";

//     //     var textNodeSelected = selectedNode.textContent.trim();
//     //     sendData("SET_DELETE_DEVICE", textNodeSelected);
//     //     selectedNode.remove();
//     // }
//     // else {
//     //     networkErrorLabel.style.color = "#C30101";
//     //     networkErrorLabel.innerHTML = "<b> Select a node from network nodes! </b>";
//     //     networkErrorLabel.style.visibility = "visible";
//     // }
// }

function delDevice() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var networkErrorLabel = iframeDocument.getElementById('networkError');
    
    // Seleccionar el nodo marcado en la lista de Network Nodes
    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
    
    if (selectedNode) {
        networkErrorLabel.style.visibility = "hidden";

        var nodeId = selectedNode.textContent.trim().split("-")[0]; // Obtener ID del nodo
        console.log("Enviando comando SET_DELETE_DEVICE para nodeID:", nodeId);
        // Enviar comando al embebido para eliminar el nodo
        sendData("SET_DELETE_DEVICE", nodeId);
        
        // Esperar confirmación antes de eliminarlo de la interfaz
        setTimeout(() => {
            selectedNode.remove();
            console.log("Nodo eliminado de la interfaz: " + nodeId);
        }, 1000);
    } else {
        networkErrorLabel.style.visibility = "visible";
        networkErrorLabel.innerText = "No device selected";
    }
}

function delAllDevices() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var networkErrorLabel = iframeDocument.getElementById('networkError');
    
    // Seleccionar la lista de nodos
    var networkNodesList = iframeDocument.getElementById('networkNodesList');
    
    console.log("Enviando comando SET_DELETE_ALL_DEVICES");
    // Enviar comando al embebido para eliminar los nodos
    sendData("SET_DELETE_ALL_DEVICES", "");

    networkErrorLabel.style.visibility = "hidden";
    networkNodesList.innerHTML = "";
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

function addGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var groupErrorLabel2 = iframeDocument.getElementById('groupError2');

    var inputField = iframeDocument.getElementById("newGroupName");
    var groupName = inputField.value.trim();

    var groupList = iframeDocument.getElementById('groupList');
    var groupList2 = iframeDocument.getElementById('groupList2');

    if (groupName !== "") {
        var exists = false;
        for (var i = 0; i < groupList.options.length; i++) {
            var optionText = groupList.options[i].textContent.trim();
            if(optionText.toLowerCase() === groupName.toLowerCase()) {
                exists = true;
                break;
            }
        }

        if(!exists) {
            groupErrorLabel2.style.visibility = "hidden";
        
            inputField.value = "";
            groupList.innerHTML = '<option value="-"> ---- </option>';
            groupList2.innerHTML = '<option value="-"> ---- </option>';
            sendData("SET_ADD_A_GROUP", groupName);
        }
        else {
            groupErrorLabel2.style.color = "#C30101";
            groupErrorLabel2.innerHTML = "<b> Group already exists! </b>";
            groupErrorLabel2.style.visibility = "visible";
        }
    }
    else {
        groupErrorLabel2.style.color = "#C30101";
        groupErrorLabel2.innerHTML = "<b> Write a name! </b>";
        groupErrorLabel2.style.visibility = "visible";
    }
}

function delGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var groupErrorLabel2 = iframeDocument.getElementById('groupError2');

    var groupList = iframeDocument.getElementById('groupList');
    var groupList2 = iframeDocument.getElementById('groupList2');
    var groupSelected = groupList2.options[groupList2.selectedIndex].value;

    if (groupSelected != '-') {
        groupErrorLabel2.style.visibility = "hidden";

        groupList.innerHTML = '<option value="-"> ---- </option>';
        groupList2.innerHTML = '<option value="-"> ---- </option>';
        sendData("SET_DEL_A_GROUP", groupSelected);
    }
    else {
        groupErrorLabel2.style.color = "#C30101";
        groupErrorLabel2.innerHTML = "<b> Pick a group! </b>";
        groupErrorLabel2.style.visibility = "visible";
    }
}

function maxButton() 
{
    sendData("SET_MAX", addressClicked);
}

function minButton() 
{
    sendData("SET_MIN", addressClicked);
}

function offButton() 
{
    sendData("SET_OFF", addressClicked);
}

function identifyButton() 
{
    sendData("SET_IDENTIFY", addressClicked);
}

function resetButton() 
{
    sendData("SET_RESET", addressClicked);
}

function rebootButton() 
{
    sendData("SET_REBOOT", addressClicked);
}

function sliderInput() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var sliderValue = iframeDocument.getElementById("lvlSlider").value;
    var message = addressClicked + ' ' + sliderValue;
    sendData("SET_ACTUAL_LVL", message);
}

function facSettingsButton() 
{
    sendData("SET_FACTORY_SETTINGS", addressClicked);
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

    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;
    var testErrorLabel = iframeDocument.getElementById('testError');

    if (groupSelected == '-') 
    {
        testErrorLabel.style.color = "#C30101";
        testErrorLabel.innerHTML = "<b> Pick a group! </b>";
        testErrorLabel.style.visibility = "visible";
    } 
    else 
    {
        var message = groupSelected + ' ';

        var toggleFunction = iframeDocument.getElementById('toggleFunction');
        message += (toggleFunction.checked ? "1" : "0") + ' ';

        var monday = iframeDocument.getElementById('monday');      
        var tuesday = iframeDocument.getElementById('tuesday');    
        var wednesday = iframeDocument.getElementById('wednesday');
        var thursday = iframeDocument.getElementById('thursday');  
        var friday = iframeDocument.getElementById('friday');      
        var saturday = iframeDocument.getElementById('saturday');  
        var sunday = iframeDocument.getElementById('sunday');      
        var functionTime = iframeDocument.getElementById('functionTimePicker').value;

        if (!toggleFunction.checked || monday.checked || tuesday.checked || wednesday.checked || thursday.checked || friday.checked || saturday.checked || sunday.checked) {
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

        var toggleDuration = iframeDocument.getElementById('toggleDuration');
        message += (toggleDuration.checked ? "1" : "0") + ' ';

        var periodicityList = iframeDocument.getElementById('periodicityList');
        var periodicitySelected = periodicityList.options[periodicityList.selectedIndex].value;
        var durationDate = iframeDocument.getElementById('durationDatePicker').value;
        var durationTime = iframeDocument.getElementById('durationTimePicker').value;

        if (!toggleDuration.checked || periodicitySelected != '-') {
            if (periodicitySelected === '-')
                message += '0' + ' ' + durationDate + ' ' + durationTime;
            else
                message += periodicitySelected + ' ' + durationDate + ' ' + durationTime;
        }
        else {
            testErrorLabel.style.color = "#C30101";
            testErrorLabel.innerHTML = "<b> Select periodicity! </b>";
            testErrorLabel.style.visibility = "visible";
            return;
        }

        testErrorLabel.style.color = "#4682b4";
        testErrorLabel.innerHTML = "<b> Test changed! </b>";
        testErrorLabel.style.visibility = "visible";
        sendData("SET_TEST", message);
    }
}

function loadTests()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var select = iframeDocument.getElementById('groupList');
    var group = select.value;
    
    if(group !== "-")
        sendData("GET_TEST", group);
    else
    {
        var toggleFunction = iframeDocument.getElementById('toggleFunction'); toggleFunction.checked = false;
        var toggleDuration = iframeDocument.getElementById('toggleDuration'); toggleDuration.checked = false;

        var monday = iframeDocument.getElementById('monday'); monday.checked = false;
        var tuesday = iframeDocument.getElementById('tuesday'); tuesday.checked = false;
        var wednesday = iframeDocument.getElementById('wednesday'); wednesday.checked = false;
        var thursday = iframeDocument.getElementById('thursday'); thursday.checked = false;
        var friday = iframeDocument.getElementById('friday'); friday.checked = false;
        var saturday = iframeDocument.getElementById('saturday'); saturday.checked = false;
        var sunday = iframeDocument.getElementById('sunday'); sunday.checked = false;

        var periodicityList = iframeDocument.getElementById('periodicityList'); periodicityList.selectedIndex = 0;

        parent.sendData("GET_DATE_TIME", "");
    }

    var testErrorLabel = iframeDocument.getElementById('testError'); testErrorLabel.innerHTML = " ‎ ";
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

function clearAllData()
{
    sendData("SET_CLEAR_ALL_DATA", "");
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

function openNodeControl(button)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');
    
    var popupText = popup.querySelector('h3');

    var device = button.getAttribute('data-device');
    var serial = button.getAttribute('data-serial');

    popupText.textContent = "A" + device + " [" + serial + "]";
    addressClicked = device;

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

function requestDevicesAndFailuresCount() {
    sendData("GET_DEVICES_COUNT", "");
    sendData("GET_FAILURES_COUNT", "");
}

function isAnExistingDevice(i) {
    sendData("GET_IS_CONFIG", i);
}

function loadGroups() {
    sendData("GET_GROUPS", "");
}

function loadGroupInfo(groupAddress) {
    sendData("GET_GROUP_INFO", groupAddress);
}

function sendPowerOnLevel(groupAddress, powerOnLevel){
    sendData("SET_POWER_ON_LVL", groupAddress + "_" + powerOnLevel)
}