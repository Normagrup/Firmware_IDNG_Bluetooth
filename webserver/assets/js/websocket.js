var socket = new WebSocket("ws://" + window.location.hostname + ":4322");
var addressClicked = 0;
var nodesScanned = 0;
var nodesAdded = 0;
var isStoppingCommission = false;
var netKey;
var antennaID;

socket.onopen = function(event) { console.log('WebSocket connection established.'); };

socket.onerror = function(error) { console.error('WebSocket error:', error); };

socket.onmessage = function(event) {
    processReceivedData(event.data);
}

function processAlertCommission(value)
{
    alert(value);
}

function processAlertLineScanning(value)
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

    // Para actualizar el valor del campo general para la fecha y hora
    var navDateTimeElem = document.getElementById("antennaDateTime");
    if (navDateTimeElem) {
        navDateTimeElem.textContent = dateString + " ~ " + timeString + "h";
    }
}

function addDeviceToScannedList(value) 
{
    var parts = value.split("_");
    var uuid = parts[0];
    var counterIncrement = (parts[1] === "true");

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    if(scannedDevicesList) {
        var devices = scannedDevicesList.getElementsByTagName('span');

        // Evitar duplicados
        for (var i = 0; i < devices.length; i++) {
            if (devices[i].textContent === uuid) { return; }
        }

        var newScanned = document.createElement('li');
        newScanned.setAttribute('class', 'deviceScanned');
        newScanned.setAttribute('onclick', 'parent.selectDevice(this)');

        var textScanned = iframeDocument.createElement('span');
        textScanned.textContent = uuid;
        textScanned.style.pointerEvents = 'none';

        var addButton = iframeDocument.createElement('button');
        addButton.textContent = "ADD";
        addButton.setAttribute('class', 'deviceAddButton');
        addButton.style.backgroundColor = "#4682b4";
        addButton.onclick = function(e) {
            e.stopPropagation();

            selectDevice(newScanned);
            addDevicePrev(textScanned.textContent);
        };

        newScanned.style.display = 'flex';
        newScanned.style.justifyContent = 'space-between';
        newScanned.style.alignItems = 'center';

        var buttonContainer = iframeDocument.createElement('div');
        buttonContainer.style.display = 'flex';
        buttonContainer.style.gap = '5px';
        buttonContainer.style.marginLeft = 'auto';

        buttonContainer.appendChild(addButton);

        newScanned.appendChild(textScanned);
        newScanned.appendChild(buttonContainer);

        scannedDevicesList.appendChild(newScanned);
    }

    if(counterIncrement) { nodesScanned++; }

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;

    updateAddReplaceScanRelayButtons();
}

function confirmStartScan(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popupScanning');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupHeader = popup.querySelector('h2');
    popupHeader.textContent = "Scan in progress...";
    
    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    setTimeout(function() {
        popup.style.visibility = "hidden";
        popupOverlay.style.visibility = "hidden";
    }, 12000); // 12 segundos (el escaneo dura 10)
}

function confirmStartCommission(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupHeader = popup.querySelector('h2');
    popupHeader.textContent = "Automatic commission in progress...";

    nodesAdded = 0; nodesScanned = 0;
    var labelCommissionNodes = iframeDocument.getElementById('labelCommissionNodes');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;

    var stopButton = iframeDocument.getElementById('stopCommissionButton');
    stopButton.classList.remove('button-disabled');

    var logCommission = iframeDocument.getElementById('logCommission');
    logCommission.innerHTML = "";
    
    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
}

function startAddingDevices(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var devices = scannedDevicesList.getElementsByTagName('span');

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

    var popup = iframeDocument.getElementById('popupAddDevice');
    var loader = popup.querySelector('.loader');
    loader.style.animation = "none";

    var logAddManualList = iframeDocument.getElementById('logAddManual');

    if(logAddManualList) {
        var newEntry = iframeDocument.createElement('li');

        var closeButton = iframeDocument.createElement('button');
        closeButton.textContent = "Close popup";
        closeButton.onclick = function () {
            closeWirelessPopup();
        };
        newEntry.appendChild(closeButton);
        
        logAddManualList.insertBefore(newEntry, logAddManualList.firstChild);
    }

    setTimeout(function() {
        var networkNodesList = iframeDocument.getElementById("networkNodesList");
        networkNodesList.innerHTML = "";

        setTimeout(function() {
            sendData("SET_LOAD_NODES", "");

            setTimeout(function() {
                sendData("SET_STORED_SCANNED_DEVICES", "");
            }, 200);
        }, 200);
    }, 200);
}

function addDeviceToNetworkList(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    if(scannedDevicesList) {
        var selectedNode = iframeDocument.querySelector('#scannedDevicesList li.selectedDevice');
        // Si se hace el add desde el MANUAL
        if(selectedNode) {
            selectedNode.remove();
        }
        // Si se hace el add desde el COMMISSION
        else {
            var devices = scannedDevicesList.getElementsByTagName('li');
            var firstDevice = devices[0];
            if (firstDevice) { firstDevice.remove(); }
        }
    }

    var parts = value.split("_");
    var nodeNetAddress = parts[0];
    var serialNumber = parts[1];
    var relayStatus = (parts[2] === "relayOn");
    var counterIncrement = (parts[3] === "true");
    
    var networkNodesList = iframeDocument.getElementById('networkNodesList');
    if(networkNodesList) {
        var newNode = iframeDocument.createElement('li');
        newNode.setAttribute('class', 'deviceNetwork');
        newNode.setAttribute('onclick', 'parent.selectDevice(this)');

        var textNode = iframeDocument.createElement('span');
        textNode.textContent = "Node " + nodeNetAddress + " - [" + serialNumber + "]";
        textNode.style.pointerEvents = 'none';

        var replaceButton = iframeDocument.createElement('button');
        replaceButton.textContent = "R";
        replaceButton.setAttribute('class', 'deviceReplaceButton');
        replaceButton.style.backgroundColor = "#4682b4";
        replaceButton.onclick = function(e) {
            e.stopPropagation();

            selectDevice(newNode);
            showReplacePopup();
        };

        var scanButton = iframeDocument.createElement('button');
        scanButton.textContent = "SCAN";
        scanButton.setAttribute('class', 'deviceScanButton');
        scanButton.style.backgroundColor = "#4682b4";
        scanButton.onclick = function(e) {
            e.stopPropagation();

            selectDevice(newNode);
            scanFromNode(textNode.textContent);
        };

        var relayButton = iframeDocument.createElement('button');
        relayButton.textContent = "RELAY";
        relayButton.setAttribute('class', 'deviceRelayButton');
        relayButton.style.backgroundColor = relayStatus ? "#4682b4" : "gray";
        relayButton.onclick = function(e) {
            e.stopPropagation();

            selectDevice(newNode);
            if(relayButton.style.backgroundColor == "gray")
                sendData("SET_RELAY_MODE", nodeNetAddress + "_" + "1");
            else
                sendData("SET_RELAY_MODE", nodeNetAddress + "_" + "0");
        };

        newNode.style.display = 'flex';
        newNode.style.justifyContent = 'space-between';
        newNode.style.alignItems = 'center';

        var buttonContainer = iframeDocument.createElement('div');
        buttonContainer.style.display = 'flex';
        buttonContainer.style.gap = '5px';
        buttonContainer.style.marginLeft = 'auto';

        buttonContainer.appendChild(replaceButton);
        buttonContainer.appendChild(scanButton);
        buttonContainer.appendChild(relayButton);

        newNode.appendChild(textNode);
        newNode.appendChild(buttonContainer);

        networkNodesList.appendChild(newNode);
    }

    if(counterIncrement) { nodesAdded++; }

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;

    updateAddReplaceScanRelayButtons();
}

function processDeviceError(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
    var popupAdd = iframeDocument.getElementById('popupAddDevice');
    var popupReplace = iframeDocument.getElementById('popupReplace');

    // Si sale DEVICE ERROR durante commissioning
    if(popup.style.visibility == "visible") {
        var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
        if(scannedDevicesList) {
            var devices = scannedDevicesList.getElementsByTagName('li');
            var firstDevice = devices[0];
            firstDevice.remove();
        }
        nodesScanned--;

        var labelCommissionNodes = popup.querySelector('label');
        labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;
    }
    // Si sale DEVICE ERROR durante add manual
    else if(popupAdd.style.visibility == "visible") {
        var selectedNode = iframeDocument.querySelector('#scannedDevicesList li.selectedDevice');
        selectedNode.remove();

        var loader = popupAdd.querySelector('.loader');
        loader.style.animation = "none";

        var logAddManualList = iframeDocument.getElementById('logAddManual');

        if(logAddManualList) {
            var newEntry1 = iframeDocument.createElement('li');

            var closeButton = iframeDocument.createElement('button');
            closeButton.textContent = "Close popup";
            closeButton.onclick = function () {
                closeWirelessPopup();
            };
            newEntry1.appendChild(closeButton);
            
            logAddManualList.insertBefore(newEntry1, logAddManualList.firstChild);
        }
    }
    // Si sale DEVICE ERROR durante replacing
    else if(popupReplace.style.visibility == "visible") {
        var selectedNode = iframeDocument.querySelector('#scannedDevicesList li.selectedDevice');
        selectedNode.remove();

        var loader = popupReplace.querySelector('.loader');
        loader.style.animation = "none";

        var logReplaceList = iframeDocument.getElementById('logReplace');

        if(logReplaceList) {
            var newEntry2 = iframeDocument.createElement('li');

            var closeButton = iframeDocument.createElement('button');
            closeButton.textContent = "Close popup";
            closeButton.onclick = function () {
                closeWirelessPopup();
            };
            newEntry2.appendChild(closeButton);
            
            logReplaceList.insertBefore(newEntry2, logReplaceList.firstChild);
        }
    }
}

function processLogCommissionEntry(value)
{
    var parts = value.split("_");
    var content = parts[0];
    var type = parts[1];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    
    var logCommissionList = iframeDocument.getElementById('logCommission');

    if(logCommissionList) {
        var firstEntry1 = logCommissionList.firstChild;
        if(!firstEntry1 || firstEntry1.textContent != content) {
            var newEntry1 = iframeDocument.createElement('li');
            newEntry1.textContent = content;
            if(type == "ERROR") { newEntry1.style.color = "#C30101"; }
            logCommissionList.insertBefore(newEntry1, firstEntry1);
        }
        else if(firstEntry1 && firstEntry1.textContent == content) {
            console.log("ENTRADA DUPLICADA: " + content);
        }
    }

    var logAddManualList = iframeDocument.getElementById('logAddManual');

    if(logAddManualList) {
        var firstEntry2 = logAddManualList.firstChild;
        if(!firstEntry2 || firstEntry2.textContent != content) {
            var newEntry2 = iframeDocument.createElement('li');
            newEntry2.textContent = content;
            if(type == "ERROR") { newEntry2.style.color = "#C30101"; }
            logAddManualList.insertBefore(newEntry2, firstEntry2);
        }
        else if(firstEntry2 && firstEntry2.textContent == content) {
            console.log("ENTRADA DUPLICADA: " + content);
        }
    }

    var logReplaceList = iframeDocument.getElementById('logReplace');

    if(logReplaceList) {
        var firstEntry3 = logReplaceList.firstChild;
        if(!firstEntry3 || firstEntry3.textContent != content) {
            var newEntry3 = iframeDocument.createElement('li');
            newEntry3.textContent = content;
            if(type == "ERROR") { newEntry3.style.color = "#C30101"; }
            logReplaceList.insertBefore(newEntry3, firstEntry3);
        }
        else if(firstEntry3 && firstEntry3.textContent == content) {
            console.log("ENTRADA DUPLICADA: " + content);
        }
    }
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
    var functionalTestButton = iframeDocument.querySelector('button[onclick="parent.funcTestButton()"]');
    var durationTestButton = iframeDocument.querySelector('button[onclick="parent.durTestButton()"]');
    var stopButton = iframeDocument.querySelector('button[onclick="parent.stopButton()"]');

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
    var actualLvlNum = parseFloat(actualLvl).toFixed(0);

    var lvlSlider = iframeDocument.getElementById("lvlSlider")
    lvlSlider.value = actualLvlNum;

    lvlIcon.innerHTML = "<b>" + actualLvlNum + "%" + "</b>";
    lvlIcon.style.background = "linear-gradient(to top, #bcf4f7 " + actualLvlNum + "%, #fff " + actualLvlNum + "%)";

    updateAllDisplayedButtons();
    requestDevicesAndFailuresCount();

    if (communicationFailure != 0) { comIcon.style.backgroundImage = "url('images/comIconOnFail.png')"; }
    else { comIcon.style.backgroundImage = "url('images/comIcon.png')"; }

    if (deviceType == "1") { 
        deviceTypeIcon.src = "images/emergencyLightIcon.png";
        autonomyIcon.classList.remove('dark-filter');
        batteryIcon.classList.remove('dark-filter');
        emergencyIcon.classList.remove('dark-filter');
        functionalTestButton.classList.remove('button-disabled');
        durationTestButton.classList.remove('button-disabled');
        stopButton.classList.remove('button-disabled');
    }
    else if (deviceType == "6") { 
        deviceTypeIcon.src = "images/normalLightIcon.png"; 
        autonomyIcon.classList.add('dark-filter');
        batteryIcon.classList.add('dark-filter');
        emergencyIcon.classList.add('dark-filter');
        functionalTestButton.classList.add('button-disabled');
        durationTestButton.classList.add('button-disabled');
        stopButton.classList.add('button-disabled');
    }
    else { 
        deviceTypeIcon.src = "images/defaultLightIcon.png";
        autonomyIcon.classList.remove('dark-filter');
        batteryIcon.classList.remove('dark-filter');
        emergencyIcon.classList.remove('dark-filter');
        functionalTestButton.classList.remove('button-disabled');
        durationTestButton.classList.remove('button-disabled');
        stopButton.classList.remove('button-disabled');
    }
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

    // Procesado si el mensaje se recibe en s_groups.html o s_tests.html: Se crea el elemento en el selector
    var groupSelector = iframeDocument.getElementById('groupList');
    if(groupSelector)
    {
        var group = iframeDocument.createElement('option');
        group.value = groupAddress;
        group.textContent = groupName;

        groupSelector.appendChild(group);
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
    var functionalTestButton = iframeDocument.querySelector('button[onclick="parent.funcTestButton()"]');
    var durationTestButton = iframeDocument.querySelector('button[onclick="parent.durTestButton()"]');
    var stopButton = iframeDocument.querySelector('button[onclick="parent.stopButton()"]');

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
    var averageLvlNum = parseFloat(averageLvl).toFixed(0);

    var lvlSlider = iframeDocument.getElementById("lvlSlider")
    lvlSlider.value = averageLvlNum;

    lvlIcon.innerHTML = "<b>" + averageLvlNum + "</b>";
    lvlIcon.style.background = "linear-gradient(to top, #bcf4f7 " + averageLvlNum + "%, #fff " + averageLvlNum + "%)";

    updateAllDisplayedButtons();
    requestDevicesAndFailuresCount();

    if (comFailures != 0) { 
        comIcon.style.backgroundImage = "url('images/comIconOnFail.png')";

        var comFailuresCountSpan = iframeDocument.createElement("span");
        comFailuresCountSpan.classList.add("fail-count-badge");
        comFailuresCountSpan.textContent = comFailures;
        comIcon.appendChild(comFailuresCountSpan);
    }
    else { comIcon.style.backgroundImage = "url('images/comIcon.png')"; }

    if (addressClicked == 49152) { 
        autonomyIcon.classList.add('dark-filter');
        batteryIcon.classList.add('dark-filter');
        emergencyIcon.classList.add('dark-filter');
        functionalTestButton.classList.add('button-disabled');
        durationTestButton.classList.add('button-disabled');
        stopButton.classList.add('button-disabled');
    }
    else { 
        autonomyIcon.classList.remove('dark-filter');
        batteryIcon.classList.remove('dark-filter');
        emergencyIcon.classList.remove('dark-filter');
        functionalTestButton.classList.remove('button-disabled');
        durationTestButton.classList.remove('button-disabled');
        stopButton.classList.remove('button-disabled');
    }
}

function processGroupInfoWithPOL(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var groupSelector = iframeDocument.getElementById('groupListPowerOnLevel');
    groupSelector.innerHTML = "<option value='-'> ---- </option>";

    var groups = value.split("#");

    for(var i = 0; i < groups.length; i++) {
        var parts = groups[i].split("_");
        var groupAddress = parts[0];
        var groupName = parts[1];
        var groupPOL = parts[2];

        // Actualizar fila en la tabla
        var tableGroupName = iframeDocument.getElementById("gp" + i);
        var tablePowerOnLevel = iframeDocument.getElementById("pl" + i);

        if(groupName !== "-" && groupPOL !== "-") {
            tableGroupName.textContent = groupName;
            tablePowerOnLevel.textContent = groupPOL === "0" ? "Off" : (groupPOL === "254" ? "Max" : "Last Value");
        }
        else {
            tableGroupName.textContent = "-";
            tablePowerOnLevel.textContent = "-";
        }

        // Añadir elemento al selector de la derecha (Si lo que llega no es una entrada vacía)
        if(groups[i] != "-_-_-") 
        {
            var group = iframeDocument.createElement('option');
            group.value = groupAddress;
            group.textContent = groupName;

            groupSelector.appendChild(group);
        }
    }
}

function processGroupNode(value, included)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var parts = value.split("_");
    var nodeNetAddress = parts[0];
    var serialNumber = parts[1];

    var node = iframeDocument.createElement('li');
    node.textContent = "Node " + nodeNetAddress + " - [" + serialNumber + "]";
    node.setAttribute('class', 'deviceIncluded');
    node.setAttribute('onclick', 'parent.selectDevice(this)');

    if(included) {   
        var includedNodesList = iframeDocument.getElementById('includedNodesList');
        includedNodesList.appendChild(node);
    } 
    else {
        var notIncludedNodesList = iframeDocument.getElementById('notIncludedNodesList');
        notIncludedNodesList.appendChild(node);
    }
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
    var devices = scannedDevicesList.getElementsByTagName('span');

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

function stopCommission()
{
    isStoppingCommission = true;

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupHeader = popup.querySelector('h2');
    popupHeader.textContent = "Stopping commissioning...";

    var stopButton = iframeDocument.getElementById('stopCommissionButton');
    stopButton.classList.add('button-disabled');

    sendData("SET_STOP_ACTION", "");
}

function processEndAutoCommission(value) 
{
    isStoppingCommission = false;

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
    var popupHeader = popup.querySelector('h2');
    popupHeader.textContent = "Commission completed";

    var loader = popup.querySelector('.loader');
    loader.style.animation = "none";

    var stopButton = iframeDocument.getElementById('stopCommissionButton');
    stopButton.classList.add('button-disabled');

    var logCommissionList = iframeDocument.getElementById('logCommission');

    if(logCommissionList) {
        var newEntry = iframeDocument.createElement('li');

        var closeButton = iframeDocument.createElement('button');
        closeButton.textContent = "Close popup";
        closeButton.onclick = function () {
            closeWirelessPopup();
        };
        newEntry.appendChild(closeButton);
        
        logCommissionList.insertBefore(newEntry, logCommissionList.firstChild);
    }

    // Limpiar la lista de escaneados cuando se hace un stop forzado
    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    scannedDevicesList.innerHTML = "";

    // Limpiar la lista de nodos de la red
    var networkNodesList = iframeDocument.getElementById('networkNodesList');
    networkNodesList.innerHTML = "";

    // Recargar nodos de la red
    sendData("SET_LOAD_NODES", "");
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

function processLogData(value) {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var logs = value.split("#");

    for(var i = 0; i < 10; i++) {
        var parts = logs[i].split("|");
        var name = parts[0];
        var serial = parts[1];
        var btAddress = parts[2];
        var IP = parts[3];
        var dateTime = parts[4];
        var event = parts[5];
        var eventType = parts[6];

        // Actualizar fila en la tabla
        var tableName = iframeDocument.getElementById("log-name-" + i);
        var tableSerial = iframeDocument.getElementById("log-serial-" + i);
        var tableBtAddress = iframeDocument.getElementById("log-btaddr-" + i);
        var tableIP = iframeDocument.getElementById("log-ip-" + i);
        var tableDateTime = iframeDocument.getElementById("log-datetime-" + i);
        var tableEvent = iframeDocument.getElementById("log-event-" + i);
        var tableType = iframeDocument.getElementById("log-type-" + i);

        tableName.textContent = name;
        tableSerial.textContent = serial;
        tableBtAddress.textContent = btAddress;
        tableIP.textContent = IP;
        tableDateTime.textContent = dateTime;
        tableEvent.textContent = event;
        tableType.textContent = eventType;
    }
}

function processLogFile(value) {
    let link = document.createElement("a");
    link.href = value;
    link.download = value.split('/').pop();
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

function processIsCommissionInProgress(value)
{
    var isCommissioning = (value === "true");
    
    if(isCommissioning) {
        var iframe = document.getElementById('mainframe');
        var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

        var popup = iframeDocument.getElementById('popup');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');

        var popupHeader = popup.querySelector('h2');
        popupHeader.textContent = isStoppingCommission ? "Stopping commissioning..." : "Automatic commission in progress...";

        var labelCommissionNodes = iframeDocument.getElementById('labelCommissionNodes');
        labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;

        var stopButton = iframeDocument.getElementById('stopCommissionButton');
        if(isStoppingCommission)
            stopButton.classList.add('button-disabled');
        else
            stopButton.classList.remove('button-disabled');

        var logCommission = iframeDocument.getElementById('logCommission');
        logCommission.innerHTML = "";
        
        popup.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";
    }
}

function processIsAddManualInProgress(value)
{
    var isAddingManual = (value === "true");

    if(isAddingManual) {
        var iframe = document.getElementById('mainframe');
        var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

        var popupAdd = iframeDocument.getElementById('popupAddDevice');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');

        popupAdd.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";

        var logAddManual = iframeDocument.getElementById('logAddManual');
        logAddManual.innerHTML = "";
    }
}

function processIsLSInProgress(value)
{
    var isLS = (value === "true");

    if(isLS) {
        var iframe = document.getElementById('mainframe');
        var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

        var popupLS = iframeDocument.getElementById('popupLineScanning');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');
        var informerLabel1 = iframeDocument.getElementById('informerLabel1');
        var informerTotal = iframeDocument.getElementById('informerTotal');
        var informerLabel2 = iframeDocument.getElementById('informerLabel2');

        var btnStop1 = iframeDocument.getElementById('stopButton1');
        var btnStop2 = iframeDocument.getElementById('stopButton2');

        popupLS.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";
        informerLabel1.textContent = "Phase 1: Completed.";
        informerTotal.textContent = "FOUNDED NODES: 0";
        informerLabel2.textContent = "Phase 2: Waiting...";

        btnStop1.disabled = true;
        btnStop2.disabled = true;

        sendData("GET_LINE_SCANNED_NODES", "");
    }
}

function processIsReplacingInProgress(value)
{
    var isReplacing = (value === "true");

    if(isReplacing) {
        var iframe = document.getElementById('mainframe');
        var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

        var popupReplace = iframeDocument.getElementById('popupReplace');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');

        popupReplace.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";

        var logReplace = iframeDocument.getElementById('logReplace');
        logReplace.innerHTML = "";
    }
}

function processIsAddingManOrReplacing(value)
{
    var parts = value.split("_");
    var isAddingMan = (parts[0] === "true");
    var isReplacing = (parts[1] === "true");

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    if(isReplacing) {
        var popupReplace = iframeDocument.getElementById('popupReplace');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');

        popupReplace.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";

        var logReplace = iframeDocument.getElementById('logReplace');
        logReplace.innerHTML = "";
    }
    else if(isAddingMan) {
        var popupAdd = iframeDocument.getElementById('popupAddDevice');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');

        popupAdd.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";

        var logAddManual = iframeDocument.getElementById('logAddManual');
        logAddManual.innerHTML = "";
    }
}

function processDelOneDev(value, init)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    
    var popup = iframeDocument.getElementById('popupDelDevice');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    if(init) // Cuando empieza el borrado
    {
        var deletingDeviceLabel = iframeDocument.getElementById('deletingDeviceLabel');
        var deletingDeviceButton = iframeDocument.getElementById('deletingDeviceButton');

        deletingDeviceLabel.textContent = "Deleting the node from the network...";
        deletingDeviceButton.classList.add('button-disabled');
    }
    else // Cuando termina el borrado
    {
        var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
        selectedNode.remove();

        popup.style.visibility = "hidden";
        popupOverlay.style.visibility = "hidden";

        /**
        var networkNodesList = iframeDocument.getElementById('networkNodesList');
        networkNodesList.innerHTML = "";

        sendData("SET_LOAD_NODES", "");

        setTimeout(function() {
            popup.style.visibility = "hidden";
            popupOverlay.style.visibility = "hidden";
        }, 3000);
         */
    }
}

function processDelAllDev(value, init)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    
    var popup = iframeDocument.getElementById('popupDelAllDevices');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    if(init) // Cuando empieza el borrado
    {
        var deletingAllDevicesLabel = iframeDocument.getElementById('deletingAllDevicesLabel');
        var deletingAllDevicesButton = iframeDocument.getElementById('deletingAllDevicesButton');

        deletingAllDevicesLabel.textContent = "Deleting all the nodes from the network...";
        deletingAllDevicesButton.classList.add('button-disabled');
    }
    else // Cuando termina el borrado
    {
        var networkNodesList = iframeDocument.getElementById('networkNodesList');
        networkNodesList.innerHTML = "";

        popup.style.visibility = "hidden";
        popupOverlay.style.visibility = "hidden";
    }
}

function processAddNodeToGroup(value)
{
    var added = (value === "true");

    loadNodesLists();

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popupAddingNode');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var addLabel = iframeDocument.getElementById('addLabel');
    var addLoader = iframeDocument.getElementById('addLoader');

    addLabel.textContent = added ? "Node correctly added" : "Something went wrong..."
    addLoader.style.animation = "none";

    setTimeout(function() {
        popup.style.visibility = "hidden";
        popupOverlay.style.visibility = "hidden";

        addLabel.textContent = "Adding Node...";
        addLoader.style.animation = "spin 1.5s linear infinite";
    }, 1800);
}

function processPowerOnLevelChange(value)
{
    var parts = value.split("_");
    var groupAddress = parts[0];
    var groupPOL = parts[1];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var selectorElement = iframeDocument.querySelector('#groupListPowerOnLevel option[value="' + groupAddress + '"]');
    var groupName = selectorElement.textContent;

    console.log(groupName);

    for (let i = 0; i < 16; i++) {
        var firstCell = iframeDocument.getElementById('gp' + i);

        if (firstCell.textContent.trim() === groupName) {
            let secondCell = iframeDocument.getElementById('pl' + i);
            secondCell.textContent = groupPOL === "0" ? "Off" : (groupPOL === "254" ? "Max" : "Last Value");
            break;
        }
    }
}

function confirmShowTree(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popupTree');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";

    loadPage('arf.html');
}

function confirmSetRelay(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var parts = value.split("_");
    var nodeNetAddress = parts[0];
    var relayStatus = (parts[1] === "relayOn");
    
    var networkNodesList = iframeDocument.getElementById('networkNodesList');
    if(networkNodesList) {
        var items = networkNodesList.querySelectorAll('li.deviceNetwork');

        items.forEach(function(item) {
            var span = item.querySelector('span');
            if (span && span.textContent.startsWith("Node " + nodeNetAddress + " -")) {
                var button = item.querySelector('button.deviceRelayButton');
                if (button) {
                    button.style.backgroundColor = relayStatus ? "#4682b4" : "gray";
                }
            }
        });
    }
}

function processMasterAddressGet(value)
{
    antennaID = value - 31767; // variable global

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var decimalValue = parseInt(value, 10);
    var id = decimalValue - 31767;

    var inputAntennaID = iframeDocument.getElementById('antennaID');
    inputAntennaID.value = id;
}

function processFailComCycles(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var inputFailComCycles = iframeDocument.getElementById('failComCycles');
    inputFailComCycles.value = value;
}

function processConfirmStartLineScanning(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popupLineScanning');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var informerLabel1 = iframeDocument.getElementById('informerLabel1');
    var informerTotal = iframeDocument.getElementById('informerTotal');
    var informerLabel2 = iframeDocument.getElementById('informerLabel2');

    var btnStop1 = iframeDocument.getElementById('stopButton1');
    var btnStop2 = iframeDocument.getElementById('stopButton2');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
    informerLabel1.textContent = "Phase 1: Starting...";
    informerTotal.textContent = "FOUNDED NODES: 0";
    informerLabel2.textContent = "Phase 2: Waiting...";

    btnStop1.disabled = false;
    btnStop2.disabled = true;
}

function processConfirmEndLineScanning(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popupLineScanning');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";
}

function processLSInfo(value)
{
    var parts = value.split("_");
    var actualNode = parts[0];
    var phase = parts[1];

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var informerLabel1 = iframeDocument.getElementById('informerLabel1');
    var informerLabel2 = iframeDocument.getElementById('informerLabel2');

    var btnStop1 = iframeDocument.getElementById('stopButton1');
    var btnStop2 = iframeDocument.getElementById('stopButton2');

    if(phase == "1") {
        informerLabel1.textContent = "Phase 1: Scanning Address " + actualNode;
        btnStop1.disabled = false;
    }
    else if(phase == "2") {
        informerLabel2.textContent = "Phase 2: Confirming Address " + actualNode;
        btnStop2.disabled = false;
    }
    else if(phase == "0") {
        informerLabel1.textContent = "Phase 1: Completed.";
        btnStop1.disabled = true;
        btnStop2.disabled = false;
    }
}

function processLSFounded(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var informerTotal = iframeDocument.getElementById('informerTotal');
    if(informerTotal)
        informerTotal.textContent = "FOUNDED NODES: " + value;
}

function processConfirmEndClearAll(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";
}

function processNetKeyGet(value)
{
    netKey = value; // variable global

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var netKeySelector = iframeDocument.getElementById("netKey");
    netKeySelector.value = value;

    var editNetKey = iframeDocument.getElementById("editNetKey");

    if(value == "16") { // si es la Custom NetKey
        editNetKey.disabled = false;

        for(var i = 0; i < 16; i++) {
            const input = iframeDocument.getElementById(`netKeyByte${i}`);
            input.value = "**";
        }
    }
    else { // si es una NetKey por defecto
        editNetKey.disabled = true;
    }
}

function processReplacing(value, init)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popupReplace = iframeDocument.getElementById("popupReplace");
    var popupOverlay = iframeDocument.getElementById("popupOverlay");
    var logReplace = iframeDocument.getElementById("logReplace");

    if(init) // Cuando empieza el replace
    {
        var popupReplacePrev = iframeDocument.getElementById("popupReplaceDevices");
        popupReplacePrev.style.visibility = "hidden";

        popupReplace.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";
        logReplace.innerHTML = "";
    }
    else // Cuando termina el replace
    {
        var loader = popupReplace.querySelector('.loader');
        loader.style.animation = "none";

        if(logReplace) {
            var newEntry = iframeDocument.createElement('li');

            var closeButton = iframeDocument.createElement('button');
            closeButton.textContent = "Close popup";
            closeButton.onclick = function () {
                closeWirelessPopup();
            };
            newEntry.appendChild(closeButton);
            
            logReplace.insertBefore(newEntry, logReplace.firstChild);
        }

        setTimeout(function() {
            var networkNodesList = iframeDocument.getElementById("networkNodesList");
            networkNodesList.innerHTML = "";

            setTimeout(function() {
                sendData("SET_LOAD_NODES", "");

                setTimeout(function() {
                    sendData("SET_STORED_SCANNED_DEVICES", "");
                }, 200);
            }, 200);
        }, 200);
    }
}

function processReceivedData(data) 
{
    var dataArray = data.split('@');
    var type = dataArray[0];
    var value = dataArray[1];
    
    if(type == 'ALERT_COMMISSION') { processAlertCommission(value); }
    else if (type == 'ALERT_LINE_SCANNING') { processAlertLineScanning(value); }
    else if (type == 'LOG_IN_INFO') { processLoginInfo(value); }
    else if (type == 'INTERFACES_INFO') { processInterfacesInfo(value); }
    else if (type == 'IPCONFIG_INFO') { processIPConfigInfo(value); }
    else if (type == 'DATE_TIME_INFO') { processDateTimeInfo(value); }
    else if (type == 'SCANNED_DEVICE') { addDeviceToScannedList(value); }
    else if (type == 'CONFIRM_START_SCAN') { confirmStartScan(value); }
    else if (type == 'CONFIRM_START_COMMISSION') { confirmStartCommission(value); }
    else if (type == 'START_ADDING_DEVICES') { startAddingDevices(value); }
    else if (type == 'CONFIRM_ADDING_DEVICE') { confirmAddingDevice(value); }
    else if (type == 'ADDED_DEVICE') { addDeviceToNetworkList(value); }
    else if (type == 'DEVICE_ERROR') { processDeviceError(value); }
    else if (type == 'LOG_COMMISSION_ENTRY') { processLogCommissionEntry(value); }
    else if (type == 'NODE_INFO') { processNodeInfo(value); }
    else if (type == 'GROUP_NAME_AND_ADDRESS') { processGroupBasicInfo(value); }
    else if (type == "GROUP_INFO") { processGroupInfo(value); }
    else if (type == "GROUP_WITH_POL") { processGroupInfoWithPOL(value); }
    else if (type == "GROUP_NODE_INCLUDED") { processGroupNode(value, true); }
    else if (type == "GROUP_NODE_NOT_INCLUDED") { processGroupNode(value, false); }
    else if (type == "TEST_DATA") { processTestData(value); }
    else if (type == "DEVICES_COUNTER") { processDevicesCounter(value); }
    else if (type == "FAILURES_COUNTER") { processFailuresCounter(value); }
    else if (type == 'END_NODE_CONFIG') { processEndNodeConfiguration(value); }
    else if (type == 'END_AUTO_COMMISSION') { processEndAutoCommission(value); }
    else if (type == 'FACTORY_ID_WROTE') { processFactoryIDWrote(value); }
    else if (type == 'DALI_TESTED') { processDaliTested(value); }
    else if (type == 'RECORDED_DEVICE') { processRecordedDevice(value); }
    else if (type == 'IS_CONFIG') { processIsConfig(value); }
    else if (type == 'LOG_DATA') { processLogData(value); }
    else if (type == 'LOG_FILE') { processLogFile(value); }
    else if (type == "IS_COMMISSION_IN_PROGRESS") { processIsCommissionInProgress(value); }
    else if (type == "IS_ADD_MANUAL_IN_PROGRESS") { processIsAddManualInProgress(value); }
    else if (type == "IS_LS_IN_PROGRESS") { processIsLSInProgress(value); }
    else if (type == "IS_REPLACING_IN_PROGRESS") { processIsReplacingInProgress(value); }
    else if (type == "IS_ADDING_MAN_OR_REPLACING") { processIsAddingManOrReplacing(value); }
    else if (type == 'CONFIRM_START_DEL_ONE_DEV') { processDelOneDev(value, true); }
    else if (type == 'CONFIRM_END_DEL_ONE_DEV') { processDelOneDev(value, false); }
    else if (type == 'CONFIRM_START_DEL_ALL_DEV') { processDelAllDev(value, true); }
    else if (type == 'CONFIRM_END_DEL_ALL_DEV') { processDelAllDev(value, false); }
    else if (type == 'CONFIRM_ADD_NODE_TO_GROUP') { processAddNodeToGroup(value); }
    else if (type == "CONFIRM_POWER_ON_LEVEL") { processPowerOnLevelChange(value); }
    else if (type == "CONFIRM_SHOW_TREE") { confirmShowTree(value); }
    else if (type == "CONFIRM_SET_RELAY") { confirmSetRelay(value); }
    else if (type == 'CONFIRM_M_ADDRESS_GET') { processMasterAddressGet(value); }
    else if (type == "FAIL_COM_CYCLES") { processFailComCycles(value); }
    else if (type == "CONFIRM_START_LS") { processConfirmStartLineScanning(value); }
    else if (type == "CONFIRM_END_LS") { processConfirmEndLineScanning(value); }
    else if (type == "LS_INFO") { processLSInfo(value); }
    else if (type == "LS_FOUNDED") { processLSFounded(value); }
    else if (type == "CONFIRM_END_CLEAR_ALL") { processConfirmEndClearAll(value); }
    else if (type == "NET_KEY_GET") { processNetKeyGet(value); }
    else if (type == "CONFIRM_START_REPLACE") { processReplacing(value, true); }
    else if (type == "CONFIRM_END_REPLACE") { processReplacing(value, false); }
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

    // Pedimos la hora actualizada para reflejarla en el header automáticamente tras cambiarla
    setTimeout(() => {
        sendData("GET_DATE_TIME", "");
    }, 500);

    var timeLabel = iframeDocument.getElementById('timeLabel');
    timeLabel.style.visibility = "visible";
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
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevices = iframeDocument.getElementById('scannedDevicesList');
    scannedDevices.innerHTML = "";

    sendData("SET_SCANNED_DEVICES", "");
}

function scanFromNode(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevices = iframeDocument.getElementById('scannedDevicesList');
    scannedDevices.innerHTML = "";

    var nodeId = value.trim().split("-")[0]; // Obtener ID del nodo

    sendData("SET_SCAN_FROM_NODE", nodeId);
}

function startCommission() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevices = iframeDocument.getElementById('scannedDevicesList');
    scannedDevices.innerHTML = "";

    sendData("SET_START_ACTION", "0");
}

function addDevice(value) 
{
    // Enviar comando al embebido para añadir el nodo
    console.log("Enviando comando SET_ADD_DEVICE para nodeID:", value);
    sendData("SET_ADD_DEVICE", value);
}

function delDevice() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    // Seleccionar el nodo marcado en la lista de Network Nodes
    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
    var nodeText = selectedNode.querySelector('span').textContent;
    var nodeId = nodeText.trim().split("-")[0]; // Obtener ID del nodo

    console.log("Enviando comando SET_DELETE_DEVICE para nodeID:", nodeId);
    // Enviar comando al embebido para eliminar el nodo
    sendData("SET_DELETE_DEVICE", nodeId);
}

function delAllDevices() { 
    console.log("Enviando comando SET_DELETE_DEVICE en BROADCAST");
    // Enviar comando al embebido para eliminar los nodos
    sendData("SET_DELETE_DEVICE", "65535");
}

function addToGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var selectedNode = iframeDocument.querySelector('#notIncludedNodesList li.selectedDevice');
    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if (!selectedNode || groupSelected == '-') { return; }

    var popup = iframeDocument.getElementById('popupAddingNode');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    var textNodeSelected = selectedNode.textContent.trim();
    var message = textNodeSelected + ' ' + groupSelected;
    sendData("SET_ADD_GROUP", message);
}

function delFromGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    
    var popup = iframeDocument.getElementById('popupDeletingNode');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var deletingNodeLabel = iframeDocument.getElementById('deletingNodeLabel');
    var deletingNodeButton = iframeDocument.getElementById('deletingNodeButton');

    var selectedNode = iframeDocument.querySelector('#includedNodesList li.selectedDevice');
    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    var textNodeSelected = selectedNode.textContent.trim();
    var message = textNodeSelected + ' ' + groupSelected;
    sendData("SET_DEL_GROUP", message);

    deletingNodeLabel.textContent = "Deleting the node from the group...";
    deletingNodeButton.classList.add('button-disabled');

    setTimeout(function() {
        popup.style.visibility = "hidden";
        popupOverlay.style.visibility = "hidden";
        loadNodesLists();
    }, 1000);
}

function addGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var groupList = iframeDocument.getElementById('groupList');

    groupList.innerHTML = getDefaultGroupsForSelector();
    sendData("SET_ADD_A_GROUP", "");

    setTimeout(function(){
        if(groupList.options.length > 0) {
            groupList.selectedIndex = groupList.options.length - 1;
        }
    }, 1000);

    setTimeout(function() {
        loadNodesLists();
    }, 1300);
}

function delGroup() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    closeGroupPopup();

    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if (groupSelected != '-') {
        groupList.innerHTML = getDefaultGroupsForSelector();
        sendData("SET_DEL_A_GROUP", groupSelected);
    }

    setTimeout(function(){
        loadNodesLists();
    }, 1000);
}

function editGroup()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var nameInput = iframeDocument.getElementById("newGroupName");
    if(nameInput.value.trim() === "") { return; }

    closeGroupPopup();

    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if (groupSelected != '-') {
        var tmpIndex = groupList.selectedIndex;
        groupList.innerHTML = getDefaultGroupsForSelector();
        sendData("SET_EDIT_A_GROUP", groupSelected + "#" + nameInput.value);
        setTimeout(function() {
            groupList.selectedIndex = tmpIndex;
        }, 1000);

        setTimeout(function() {
            loadNodesLists();
        }, 1300);
    }
}

function loadNodesLists()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var includedNodesList = iframeDocument.getElementById('includedNodesList');
    includedNodesList.innerHTML = "";

    var notIncludedNodesList = iframeDocument.getElementById('notIncludedNodesList');
    notIncludedNodesList.innerHTML = "";

    var groupList = iframeDocument.getElementById("groupList");
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if(groupSelected == "-") { return; }

    sendData("GET_GROUP_NODES", groupSelected);
}

function maxButton() 
{
    sendData("SET_MAX", addressClicked);

    if(addressClicked < 49152) {
        sendData("GET_NODE_INFO", addressClicked);
    } else {
        loadGroupInfo(transformDecimalGroupAddressIntoHexGroupAddress(addressClicked));
    }
}

function minButton() 
{
    sendData("SET_MIN", addressClicked);

    if(addressClicked < 49152) {
        sendData("GET_NODE_INFO", addressClicked);
    }
    else {
        loadGroupInfo(transformDecimalGroupAddressIntoHexGroupAddress(addressClicked));
    }  
}

function offButton() 
{
    sendData("SET_OFF", addressClicked);

    if(addressClicked < 49152) {
        sendData("GET_NODE_INFO", addressClicked);
    } else {
        loadGroupInfo(transformDecimalGroupAddressIntoHexGroupAddress(addressClicked));
    }
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

    var lvlSlider = iframeDocument.getElementById("lvlSlider");

    if(lvlSlider.value == 0) 
    {
        sendData("SET_OFF", addressClicked);

        if(addressClicked < 49152) {
            sendData("GET_NODE_INFO", addressClicked);
        } else {
            loadGroupInfo(transformDecimalGroupAddressIntoHexGroupAddress(addressClicked));
        }
    }
    else
    {
        var message = addressClicked + ' ' + lvlSlider.value;
        sendData("SET_ACTUAL_LVL", message);

        var lvlIcon = iframeDocument.getElementById('lvlIcon');
        lvlIcon.innerHTML = "<b>" + lvlSlider.value + "%" + "</b>";
        lvlIcon.style.background = "linear-gradient(to top, #bcf4f7 " + lvlSlider.value + "%, #fff " + lvlSlider.value + "%)";
    }
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
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    sendData("SET_CLEAR_ALL_DATA", "");

    var input = iframeDocument.getElementById('deleteConfirmInput');
    input.value = "";

    var button = iframeDocument.getElementById('deletingDataButton');
    button.disabled = true;

    var confirmDeleteData = iframeDocument.getElementById('confirmDeleteData');
    confirmDeleteData.textContent = "Deleting ALL data. Don't leave this screen";
}

function lineScanning()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var start = parseInt(iframeDocument.getElementById("scanStart").value);
    var end = parseInt(iframeDocument.getElementById("scanEnd").value);

    if(start >= 1 && start <= 2048 && end >= 1 && end <= 2048 && start <= end)
        sendData("LINE_SCANNING", start + "_" + end);
}

function getLogs()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var reportList = iframeDocument.getElementById('reportList');
    var reportListSelected = reportList.options[reportList.selectedIndex].value;
    var initialDatePicker = iframeDocument.getElementById('initialDatePicker');
    var finalDatePicker = iframeDocument.getElementById('finalDatePicker');

    var initialDate = new Date(initialDatePicker.value);
    var finalDate = new Date(finalDatePicker.value);
    initialDate.setHours(0, 0, 0, 0);
    finalDate.setHours(0, 0, 0, 0);

    var showButton = iframeDocument.getElementById("showLogs");

    if (reportListSelected != '-' && initialDatePicker.value && finalDatePicker.value) {
        if (initialDate <= finalDate) {
            var message = reportListSelected + ' ' + initialDatePicker.value + ' ' + finalDatePicker.value

            sendData("GET_LOGS", message);
            showButton.style.backgroundColor = "green";
            setTimeout(function () {
                showButton.style.backgroundColor = "#4682b4";
            }, 500);

            var pageLabel = iframeDocument.getElementById("pageIndicator");
            pageLabel.textContent = "Page: 1";
        }
        else {
            showButton.style.backgroundColor = "red";
            setTimeout(function () {
                showButton.style.backgroundColor = "#4682b4";
            }, 500);
        }
    }
    else {
        showButton.style.backgroundColor = "red";
        setTimeout(function () {
            showButton.style.backgroundColor = "#4682b4";
        }, 500);
    }
}

function previousLogPage() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var pageLabel = iframeDocument.getElementById("pageIndicator");
    
    if(pageLabel.textContent.trim() === "Page: 0") { return; } // Si no hay datos cargados
    if(pageLabel.textContent.trim() === "Page: 1") { return; } // Si es la primera página

    var currentPageStr = pageLabel.textContent.replace("Page:", "").trim();
    var currentPage = parseInt(currentPageStr, 10);
    currentPage--;
    pageLabel.textContent = "Page: " + currentPage;

    sendData("GET_LOGS_PAGED", currentPage);
}

function nextLogPage() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var pageLabel = iframeDocument.getElementById("pageIndicator");
    var tableNameLast = iframeDocument.getElementById("log-name-9");
    var tableSerialLast = iframeDocument.getElementById("log-serial-9");

    if(pageLabel.textContent.trim() === "Page: 0") { return; } // Si no hay datos cargados
    if(tableNameLast.textContent.trim() === "-" || tableSerialLast.textContent.trim() === "-") { return; } // Si es la última página

    var currentPageStr = pageLabel.textContent.replace("Page:", "").trim();
    var currentPage = parseInt(currentPageStr, 10);
    currentPage++;
    pageLabel.textContent = "Page: " + currentPage;

    sendData("GET_LOGS_PAGED", currentPage);
}

function downloadLogs()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var reportList = iframeDocument.getElementById('reportList');
    var reportListSelected = reportList.options[reportList.selectedIndex].value;
    var initialDatePicker = iframeDocument.getElementById('initialDatePicker');
    var finalDatePicker = iframeDocument.getElementById('finalDatePicker');

    var initialDate = new Date(initialDatePicker.value);
    var finalDate = new Date(finalDatePicker.value);
    initialDate.setHours(0, 0, 0, 0);
    finalDate.setHours(0, 0, 0, 0);

    var exportButton = iframeDocument.getElementById("exportLogs");

    if (reportListSelected != '-' && initialDatePicker.value && finalDatePicker.value) {
        if (initialDate <= finalDate) {
            var message = reportListSelected + ' ' + initialDatePicker.value + ' ' + finalDatePicker.value

            sendData("DOWNLOAD_LOGS", message);
            exportButton.style.backgroundColor = "green";
            setTimeout(function () {
                exportButton.style.backgroundColor = "#4682b4";
            }, 500);
        }
        else {
            exportButton.style.backgroundColor = "red";
            setTimeout(function () {
                exportButton.style.backgroundColor = "#4682b4";
            }, 500);
        }
    }
    else {
        exportButton.style.backgroundColor = "red";
        setTimeout(function () {
            exportButton.style.backgroundColor = "#4682b4";
        }, 500);
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

function updateAllDisplayedButtons() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    const container = iframeDocument.getElementById("node-container");
    const buttons = container.querySelectorAll('button[data-device]');
    buttons.forEach(button => {
        const device = button.getAttribute('data-device');
        isAnExistingDevice(device);
    });
}

function showTree() {
    sendData("SET_RELOAD_TREE", "");

    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var popup = iframeDocument.getElementById('popupTree');

    popupOverlay.style.visibility = "visible";
    popup.style.visibility = "visible";
}

function goToPreviousPage() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var pageLabel = iframeDocument.getElementById("page");
    
    if(pageLabel.textContent.trim() === "Page: 1") { return; }

    var currentPageStr = pageLabel.textContent.replace("Page:", "").trim();
    var currentPage = parseInt(currentPageStr, 10);
    currentPage--;
    pageLabel.textContent = "Page: " + currentPage;

    sendData("GET_POWER_ON_LVL", currentPage);
}

function goToNextPage() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var tableGroupNameLast = iframeDocument.getElementById("gp15");
    var tablePowerOnLevelLast = iframeDocument.getElementById("pl15");

    if(tableGroupNameLast.textContent.trim() === "-" || tablePowerOnLevelLast.textContent.trim() === "-") { return; }

    var pageLabel = iframeDocument.getElementById("page");

    var currentPageStr = pageLabel.textContent.replace("Page:", "").trim();
    var currentPage = parseInt(currentPageStr, 10);
    currentPage++;
    pageLabel.textContent = "Page: " + currentPage;

    sendData("GET_POWER_ON_LVL", currentPage);
}

function setPowerOnLevel(){
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var groupSelector = iframeDocument.getElementById('groupListPowerOnLevel');
    var levelSelector = iframeDocument.getElementById('powerOnLevelList');
    var errorLabel = iframeDocument.getElementById('powerOnError');
    
    if (groupSelector.value === "-") {
        errorLabel.style.visibility = "visible";
    } 
    else {
        errorLabel.style.visibility = "hidden";

        sendData("SET_POWER_ON_LVL", groupSelector.value + "_" + levelSelector.value)
    }
}

function requestDateTime() {
    sendData("GET_DATE_TIME", "");
}

function syncPOL() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    
    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    sendData("SET_SYNC_POL", "");

    setTimeout(function() {
        var pageLabel = iframeDocument.getElementById("page");

        if(pageLabel) {
            var currentPageStr = pageLabel.textContent.replace("Page:", "").trim();
            var currentPage = parseInt(currentPageStr, 10);

            pageLabel.textContent = "Page: " + currentPage;

            sendData("GET_POWER_ON_LVL", currentPage);
        }

        popup.style.visibility = "hidden";
        popupOverlay.style.visibility = "hidden";
    }, 12000);
}

function setAntennaNumberAndNetKey() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var inputAntennaID = iframeDocument.getElementById('antennaID');
    var netKeySelector = iframeDocument.getElementById("netKey");

    var newAntennaID = "";
    if(inputAntennaID.value != antennaID) {
        newAntennaID = inputAntennaID.value;
    }

    var newNetKey = "";
    if((netKeySelector.value != "16" && netKeySelector.value != netKey) || (netKeySelector.value == "16" && (netKeySelector.value != netKey || iframeDocument.getElementById("netKeyByte0").value != "**"))) {
        if(netKeySelector.value != "16") {
            newNetKey = netKeySelector.value;
        }
        else {
            for(var i = 0; i < 16; i++) {
                const input = iframeDocument.getElementById(`netKeyByte${i}`);
                const value = input.value.trim().toUpperCase();
                newNetKey += value;
            }
        }
    }

    if((inputAntennaID.value < 1 || inputAntennaID.value > 1000) || (netKeySelector.value == "16" && iframeDocument.getElementById("netKeyByte0").value == "")) {
        return; // forzamos stop por error de rango de ID o falta de netkey
    }

    var isTrue = confirm("You are going to reboot the IDNG-Blue! Are you sure?")
    if (isTrue) {
        sendData("SET_MASTER_ADDR_AND_NETKEY", newAntennaID + "_" + newNetKey);

        setTimeout(function () {
            sendData("SET_REBOOT_DEVICE", " ");
            logoutApp();
            window.location.href = "http://" + window.location.hostname;
        }, 2000);
    }
}

function saveCycles()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var inputFailComCycles = iframeDocument.getElementById('failComCycles');

    var saveButton = iframeDocument.getElementById('saveCycles');

    if(inputFailComCycles.value > 0) {
        sendData("SET_FAILCOM_CYCLES", inputFailComCycles.value);

        if (saveButton) {
            saveButton.style.backgroundColor = "green";

            setTimeout(function () {
                saveButton.style.backgroundColor = "#4682b4";
            }, 500);
        }
    }
    else {
        if (saveButton) {
            saveButton.style.backgroundColor = "red";

            setTimeout(function () {
                saveButton.style.backgroundColor = "#4682b4";
            }, 500);
        }
    }
}

function confirmSwap()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var position1 = parseInt(iframeDocument.getElementById('positionInput1').value, 10);
    var position2 = parseInt(iframeDocument.getElementById('positionInput2').value, 10);

    var confirmSwapButton = iframeDocument.getElementById('confirmSwapButton');

    if(position1 < 1 || position1 > 2048 || position2 < 1 || position2 > 2048 || position1 == position2) {
        confirmSwapButton.style.backgroundColor = "red";
        confirmSwapButton.style.color = "white";
        setTimeout(function () {
            confirmSwapButton.style.backgroundColor = "";
            confirmSwapButton.style.color = "";
        }, 500);
    }
    else {
        sendData("CHANGE_NODES", position1 + "_" + position2),

        confirmSwapButton.style.backgroundColor = "green";
        confirmSwapButton.style.color = "white";
        setTimeout(function () {
            confirmSwapButton.style.backgroundColor = "";
            confirmSwapButton.style.color = "";
        }, 500);

        setTimeout(function() {
            updateAllDisplayedButtons();
        }, 1000);
    }
}

function stopLS(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var btnStop1 = iframeDocument.getElementById('stopButton1');
    var btnStop2 = iframeDocument.getElementById('stopButton2');

    if(value == "1") 
    {
        btnStop1.disabled = true;
        btnStop2.disabled = false;
    } 
    else if(value == "2") 
    {
        btnStop2.disabled = true;
    }

    sendData("STOP_LS", value);
}

function replaceDevice()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var selectedIndex = iframeDocument.getElementById('deviceToReplaceSelect').selectedIndex;
    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');

    if(selectedIndex != -1 && selectedNode) {
        var newNodeUUID = iframeDocument.getElementById('deviceToReplaceSelect')[selectedIndex].value;

        var oldNodeText = selectedNode.querySelector('span').textContent;
        var oldNodeId = oldNodeText.trim().split("-")[0]; // Obtener ID del nodo

        sendData("REPLACE_NODES", newNodeUUID + "_" + oldNodeId);
    }
}