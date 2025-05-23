var socket = new WebSocket("ws://" + window.location.hostname + ":4322");
var addressClicked = 0;
var nodesScanned = 0;
var nodesAdded = 0;
var isStoppingCommission = false;

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

    // Para actualizar el valor del campo general para la fecha y hora
    var navDateTimeElem = document.getElementById("antennaDateTime");
    if (navDateTimeElem) {
        navDateTimeElem.textContent = dateString + " ~ " + timeString + "h";
    }
}

function addDeviceToScannedList(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    if(scannedDevicesList) {
        var devices = scannedDevicesList.getElementsByTagName('li');

        // Evitar duplicados
        for (var i = 0; i < devices.length; i++) {
            if (devices[i].textContent === value) { return; }
        }

        var newScanned = document.createElement('li');
        newScanned.textContent = value;
        newScanned.setAttribute('class', 'deviceScanned');
        newScanned.setAttribute('onclick', 'parent.selectDevice(this)');
        scannedDevicesList.appendChild(newScanned);
    }
    nodesScanned++;

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;
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
        }, 200);
    }, 200);
}

function addDeviceToNetworkList(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    // Si llega la info de un nodo en red y estamos durante un commissioning, 
    // se elimina uno de los elementos de scannedDevices y se añade uno a networkNodes 
    // Si llega la info de un nodo en red y no estamos durante un commissioning, 
    // simplemente se añadirá a networkNodes pero no se eliminará nada de scannedDevices (ya que estará vacía)
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

        var button = iframeDocument.createElement('button');
        button.textContent = "R";
        button.setAttribute('class', 'deviceRelayButton');
        button.style.backgroundColor = relayStatus ? "#4682b4" : "gray";
        button.onclick = function(e) {
            e.stopPropagation();

            if(button.style.backgroundColor == "gray")
                sendData("SET_RELAY_MODE", nodeNetAddress + "_" + "1");
            else
                sendData("SET_RELAY_MODE", nodeNetAddress + "_" + "0");
        };

        newNode.appendChild(textNode);
        newNode.appendChild(button);

        networkNodesList.appendChild(newNode);
    }

    if(counterIncrement) { nodesAdded++; }

    var popup = iframeDocument.getElementById('popup');
    var labelCommissionNodes = popup.querySelector('label');
    labelCommissionNodes.textContent = nodesAdded + " / " + nodesScanned;
}

function processDeviceError(value) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
    var popupAdd = iframeDocument.getElementById('popupAddDevice');

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
            var newEntry = iframeDocument.createElement('li');

            var closeButton = iframeDocument.createElement('button');
            closeButton.textContent = "Close popup";
            closeButton.onclick = function () {
                closeWirelessPopup();
            };
            newEntry.appendChild(closeButton);
            
            logAddManualList.insertBefore(newEntry, logAddManualList.firstChild);
        }
    }
}

function processLogCommissionEntry(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    
    var logCommissionList = iframeDocument.getElementById('logCommission');

    if(logCommissionList) {
        var newEntry1 = iframeDocument.createElement('li');
        newEntry1.textContent = value;
        if(value == "An error has occurred with the device...")
            newEntry1.style.color = "#C30101";
        logCommissionList.insertBefore(newEntry1, logCommissionList.firstChild);
    }

    var logAddManualList = iframeDocument.getElementById('logAddManual');

    if(logAddManualList) {
        var newEntry2 = iframeDocument.createElement('li');
        newEntry2.textContent = value;
        if(value == "An error has occurred with the device...")
            newEntry2.style.color = "#C30101";
        logAddManualList.insertBefore(newEntry2, logAddManualList.firstChild);
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

function processLogData(value){
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
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    loadNodesLists();

    var popup = iframeDocument.getElementById('popupAddingNode');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";
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
    else if (type == "IS_COMMISSION_IN_PROGRESS") { processIsCommissionInProgress(value); }
    else if (type == 'CONFIRM_START_DEL_ONE_DEV') { processDelOneDev(value, true); }
    else if (type == 'CONFIRM_END_DEL_ONE_DEV') { processDelOneDev(value, false); }
    else if (type == 'CONFIRM_START_DEL_ALL_DEV') { processDelAllDev(value, true); }
    else if (type == 'CONFIRM_END_DEL_ALL_DEV') { processDelAllDev(value, false); }
    else if (type == 'CONFIRM_ADD_NODE_TO_GROUP') { processAddNodeToGroup(value); }
    else if (type == "CONFIRM_POWER_ON_LEVEL") { processPowerOnLevelChange(value); }
    else if (type == "CONFIRM_SHOW_TREE") { confirmShowTree(value); }
    else if (type == "CONFIRM_SET_RELAY") { confirmSetRelay(value); }
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

function scanFromNode()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var networkErrorLabel = iframeDocument.getElementById('networkError');

    // Seleccionar el nodo marcado en la lista de Network Nodes
    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');

    if(selectedNode) {
        networkErrorLabel.style.visibility = "hidden";

        var scannedDevices = iframeDocument.getElementById('scannedDevicesList');
        scannedDevices.innerHTML = "";

        var nodeText = selectedNode.querySelector('span').textContent;
        var nodeId = nodeText.trim().split("-")[0]; // Obtener ID del nodo

        sendData("SET_SCAN_FROM_NODE", nodeId);
    } else {
        networkErrorLabel.style.visibility = "visible";
        networkErrorLabel.innerText = "No network node selected";
    }
}

function startCommission() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevices = iframeDocument.getElementById('scannedDevicesList');
    scannedDevices.innerHTML = "";

    sendData("SET_START_ACTION", "0");
}

function addDevice() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    // Seleccionar el nodo marcado en la lista de Scanned Devices
    var selectedDevice = iframeDocument.querySelector('#scannedDevicesList li.selectedDevice');
    var scannedUUID = selectedDevice.textContent.trim(); 

    // Enviar comando al embebido para añadir el nodo
    console.log("Enviando comando SET_ADD_DEVICE para nodeID:", scannedUUID);
    sendData("SET_ADD_DEVICE", scannedUUID);
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

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";

    var clearDataLabel = iframeDocument.getElementById('clearDataLabel');
    clearDataLabel.style.visibility = "visible";
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
    initialDate.setHours(0, 0, 0, 0);
    finalDate.setHours(0, 0, 0, 0);

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
    }, 10000);
}