function logoutApp()
{
    var logoutButton = document.getElementById("logoutButton");
    if (logoutButton) { logoutButton.remove(); }

    var networkButton = document.getElementById("networkButton");
    if (networkButton) { networkButton.remove(); }

    var settingsButton = document.getElementById("settingsButton");
    if (settingsButton) { settingsButton.remove(); }

    var loginButton = document.createElement('li');
    loginButton.id = "loginButton";
    var loginButtonLink = document.createElement('a');
    loginButtonLink.textContent = "Log in";
    loginButtonLink.onclick = openLogin;

    var dot = document.createElement('div');
    dot.className = "dot";

    loginButton.appendChild(loginButtonLink);
    loginButton.appendChild(dot);

    var loginContainer = document.getElementById("loginContainer");
    loginContainer.appendChild(loginButton);

    var summaryButton = document.getElementById("summaryButton");
    if (!summaryButton) {
        summaryButton = document.createElement('li');
        summaryButton.id = "summaryButton";
        var summaryButtonLink = document.createElement('a');
        summaryButtonLink.textContent = "Summary";
        summaryButtonLink.onclick = function() { loadPage('summary.html'); };

        var dot = document.createElement('div');
        dot.className = "dot";

        summaryButton.appendChild(summaryButtonLink);
        summaryButton.appendChild(dot);

        var menuContainer = document.getElementById("menuContainer");
        menuContainer.appendChild(summaryButton);
    }

    loadPage('summary.html');
}

function createLogoutButton(userName) 
{
    var loginButton = document.getElementById("loginButton");
    if (loginButton) { loginButton.remove(); }

    // To avoid unintentional repeated creations
    var logoutButtonExisting = document.getElementById("logoutButton");
    if (logoutButtonExisting) { return; }

    var logoutButton = document.createElement('li');
    logoutButton.id = "logoutButton";
    var logoutButtonLink = document.createElement('a');
    logoutButtonLink.textContent = userName;
    var logoutButtonMenu = document.createElement('ul');
    logoutButtonMenu.className = "drop-menu"

    var logoutReboot = document.createElement('li');
    var logoutRebootLink = document.createElement('a');
    logoutRebootLink.onclick = rebootDevice;
    logoutRebootLink.textContent = "Reboot";

    var logoutExit = document.createElement('li');
    var logoutExitLink = document.createElement('a');
    logoutExitLink.onclick = logoutApp;
    logoutExitLink.textContent = "Log out";

    logoutReboot.appendChild(logoutRebootLink);
    logoutExit.appendChild(logoutExitLink);

    logoutButtonMenu.appendChild(logoutReboot);
    logoutButtonMenu.appendChild(logoutExit);

    logoutButtonLink.appendChild(logoutButtonMenu);

    var dot = document.createElement('div');
    dot.className = "dot";

    logoutButton.appendChild(logoutButtonLink);
    logoutButton.appendChild(dot);
    
    var loginContainer = document.getElementById("loginContainer");
    loginContainer.appendChild(logoutButton);
}

function createSettingsButton() 
{
    // To avoid unintentional repeated creations
    var settingsButtonExisting = document.getElementById("settingsButton");
    if(settingsButtonExisting) { return; }

    var settingsButton = document.createElement('li');
    settingsButton.id = "settingsButton";

    var settingsButtonLink = document.createElement('a');
    settingsButtonLink.onclick = function() { loadPage('settings.html') };
    settingsButtonLink.textContent = "Settings";

    var settingsButtonMenu = document.createElement('ul');
    settingsButtonMenu.className = "drop-menu"

    var settingsIPConfig = document.createElement('li');
    var settingsTime = document.createElement('li');
    var settingsWirelessConfig = document.createElement('li');
    var settingsGroupsConfig = document.createElement('li');
    var settingsLogs = document.createElement('li');
    var settingsTests = document.createElement('li');
    var settingsPowerOnLevel = document.createElement('li');
    var settingsUpdateDevice = document.createElement('li');
    var settingsGeneralConfig = document.createElement('li');

    var settingsIPConfigLink = document.createElement('a');
    settingsIPConfigLink.onclick = function() { loadPage('s_ipconfig.html'); };
    settingsIPConfigLink.textContent = "IP Configuration";

    var settingsTimeLink = document.createElement('a');
    settingsTimeLink.onclick = function() { loadPage('s_time.html') };
    settingsTimeLink.textContent = "Time";

    var settingsWirelessConfigLink = document.createElement('a');
    settingsWirelessConfigLink.onclick = function() { loadPage('s_wireless.html') };
    settingsWirelessConfigLink.textContent = "Wireless Config";

    var settingsGroupsConfigLink = document.createElement('a');
    settingsGroupsConfigLink.onclick = function() { loadPage('s_groups.html') };
    settingsGroupsConfigLink.textContent = "Groups Config";

    var settingsLogsLink = document.createElement('a');
    settingsLogsLink.onclick = function() { loadPage('s_logs.html') };
    settingsLogsLink.textContent = "Log Results";

    var settingsTestsLink = document.createElement('a');
    settingsTestsLink.onclick = function() { loadPage('s_tests.html') };
    settingsTestsLink.textContent = "Emergency Tests";

    var settingsPowerOnLevelLink = document.createElement('a');
    settingsPowerOnLevelLink.onclick = function() { loadPage('s_power_on_level.html') };
    settingsPowerOnLevelLink.textContent = "Power On Level";

    var settingsUpdateDeviceLink = document.createElement('a');
    settingsUpdateDeviceLink.onclick = function() { loadPage('s_update.html') };
    settingsUpdateDeviceLink.textContent = "Update Device";
     
    var settingsGeneralConfigLink = document.createElement('a');
    settingsGeneralConfigLink.onclick = function() { loadPage('s_general_config.html') };
    settingsGeneralConfigLink.textContent = "General Config";

    settingsIPConfig.appendChild(settingsIPConfigLink);
    settingsTime.appendChild(settingsTimeLink);
    settingsWirelessConfig.appendChild(settingsWirelessConfigLink);
    settingsGroupsConfig.appendChild(settingsGroupsConfigLink);
    settingsLogs.appendChild(settingsLogsLink);
    settingsTests.appendChild(settingsTestsLink);
    settingsPowerOnLevel.appendChild(settingsPowerOnLevelLink);
    settingsUpdateDevice.appendChild(settingsUpdateDeviceLink);
    settingsGeneralConfig.appendChild(settingsGeneralConfigLink);

    settingsButtonMenu.appendChild(settingsIPConfig);
    settingsButtonMenu.appendChild(settingsTime);
    settingsButtonMenu.appendChild(settingsWirelessConfig);
    settingsButtonMenu.appendChild(settingsGroupsConfig);
    settingsButtonMenu.appendChild(settingsLogs);
    settingsButtonMenu.appendChild(settingsTests);
    settingsButtonMenu.appendChild(settingsPowerOnLevel);
    settingsButtonMenu.appendChild(settingsUpdateDevice);
    settingsButtonMenu.appendChild(settingsGeneralConfig);

    settingsButtonLink.appendChild(settingsButtonMenu);

    var dot = document.createElement('div');
    dot.className = "dot";

    settingsButton.appendChild(settingsButtonLink);
    settingsButton.appendChild(dot);
    
    var menuContainer = document.getElementById("menuContainer");
    menuContainer.appendChild(settingsButton);

    settingsButtonMenu.addEventListener('click', function(event) {
        event.stopPropagation();
    });
}

function createAdminMenuButtons()
{
    // To avoid unintentional repeated creations
    var networkButtonExisting = document.getElementById("networkButton");
    if(!networkButtonExisting) 
    {
        var networkButton = document.createElement('li');
        networkButton.id = "networkButton";
        var networkButtonLink = document.createElement('a');
        networkButtonLink.textContent = "Network";
        networkButtonLink.onclick = function() { 
            loadPage('network.html'); 
        };
        
        var dot = document.createElement('div');
        dot.className = "dot";

        networkButton.appendChild(networkButtonLink);
        networkButton.appendChild(dot);

        var menuContainer = document.getElementById("menuContainer");
        menuContainer.appendChild(networkButton);
    }

    createSettingsButton();
}

function loadAdminMenu()
{
    closeLoginPopup();

    createLogoutButton("admin");

    createAdminMenuButtons();

    loadPage('summary.html');

    buildOnLoadMethodAdmin();
}

function loadFactoryMenu()
{
    closeLoginPopup();

    var summaryButton = document.getElementById("summaryButton");
    if (summaryButton) { summaryButton.remove(); }

    createLogoutButton("fabrica");

    loadPage('factory.html');
}

function selectDevice(device) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    var networkNodesList = iframeDocument.getElementById('networkNodesList');

    // Botones de nodos en s_wireless.html
    if(scannedDevicesList && networkNodesList) {
        var scannedDevices = scannedDevicesList.getElementsByTagName('li');
        var networkDevices = networkNodesList.getElementsByTagName('li');

        for (var i = 0; i < scannedDevices.length; i++) { scannedDevices[i].classList.remove('selectedDevice'); }
        for (var i = 0; i < networkDevices.length; i++) { networkDevices[i].classList.remove('selectedDevice'); }

        device.classList.add('selectedDevice');
    }

    var includedNodesList = iframeDocument.getElementById('includedNodesList');
    var notIncludedNodesList = iframeDocument.getElementById('notIncludedNodesList');

    // Botones de nodos en s_groups.html
    if(includedNodesList && notIncludedNodesList) {
        var includedNodes = includedNodesList.getElementsByTagName('li');
        var notIncludedNodes = notIncludedNodesList.getElementsByTagName('li');

        for (var i = 0; i < includedNodes.length; i++) { includedNodes[i].classList.remove('selectedDevice'); }
        for (var i = 0; i < notIncludedNodes.length; i++) { notIncludedNodes[i].classList.remove('selectedDevice'); }

        device.classList.add('selectedDevice');
    }
}

function openGroupControl(button)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupText = popup.querySelector('h3');
    var groupTypeIcon = iframeDocument.getElementById('deviceTypeIcon');
	
    var groupAddress = button.getAttribute('group-address');

    // Carga de imágenes del grupo
    if (groupAddress === 'C000') { groupTypeIcon.src = "images/normalLightIcon.png"; }
    else if (groupAddress === 'C001') { groupTypeIcon.src = "images/emergencyLightIcon.png"; }
    else { groupTypeIcon.src = "images/defaultLightIcon.png"; }

    popupText.textContent = button.textContent;
    addressClicked = parseInt(groupAddress, 16);
    
    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    loadGroupInfo(groupAddress);
}

function createNodeButtons(start, end) 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var container = iframeDocument.getElementById('node-container');
    var count = 0;

    for (var i = start; i <= end; i++) {
        var button = iframeDocument.createElement('button');
        var address = i.toString(10);
        button.textContent = address;
        button.onclick = function() {
            openNodeControl(this);
        };

        button.setAttribute('data-device', i); // para que el botón "sepa" a que dispositivo pertenece
        button.classList.add("gray"); // clase por defecto que pinta el botón de gris
        isAnExistingDevice(i);

        container.appendChild(button);
        count++;
        if (count >= 64) break;
    }
}

function changeAddress() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var select = iframeDocument.getElementById('addressList');
    var subnet = select.value;
    var start = (parseInt(subnet, 10) - 1) * 64 + 1;
    var end = parseInt(subnet, 10) * 64;
    var container = iframeDocument.getElementById('node-container');
    container.innerHTML = '';

    createNodeButtons(start, end);
}

function createGroupButtons() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var container = iframeDocument.getElementById('group-container');

    for (var i = 1; i <= 4; i++) {
        var button = iframeDocument.createElement('button');

        if (i === 1) { button.textContent = "(Gr0) Lighting"; button.setAttribute('group-address', "C000"); } 
        else if (i === 2) { button.textContent = "(Gr1) Emergency"; button.setAttribute('group-address', "C001"); } 
        else if (i === 3) { button.textContent = "(Gr2) Even"; button.setAttribute('group-address', "C002"); } 
        else if (i === 4) { button.textContent = "(Gr3) Odd"; button.setAttribute('group-address', "C003"); }
        button.onclick = function() {
            openGroupControl(this);
        };
        container.appendChild(button);
    }

    loadGroups();
}

function updateDevice()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var confirmed = window.confirm("Are you sure to upload the file and reboot the device?");
    if (confirmed) {
        var fileInput = iframeDocument.getElementById('fileInput');
        var file = fileInput.files[0];

        if (!file) {
            alert("Please, select a file before updating!");
            return;
        }

        sendFile();
    }
}

function switchMode()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var toggleMode = iframeDocument.getElementById("toggleMode");
    var automaticLabel = iframeDocument.getElementById("automaticLabel");
    var manualLabel = iframeDocument.getElementById("manualLabel");
    var automaticContainer = iframeDocument.getElementById("automaticContainer");
    var manualContainer = iframeDocument.getElementById("manualContainer");
    var manualContainerSD = iframeDocument.getElementById("manualContainerScannedDevices");
    var manualContainerNN = iframeDocument.getElementById("manualContainerNetworkNodes");

    var addButtons = iframeDocument.querySelectorAll('.deviceAddButton');
    var replaceButtons = iframeDocument.querySelectorAll('.deviceReplaceButton');
    var scanButtons = iframeDocument.querySelectorAll('.deviceScanButton');
    var relayButtons = iframeDocument.querySelectorAll('.deviceRelayButton');

    var allButtons = [...addButtons, ...replaceButtons, ...scanButtons, ...relayButtons];

    if (toggleMode.checked) {
        automaticLabel.style.color = "#999";
        automaticLabel.style.fontWeight = "normal";
        automaticContainer.style.display = "none";

        manualLabel.style.color = "#4682b4";
        manualLabel.style.fontWeight = "bold";
        manualContainer.style.display = "flex";
        manualContainerSD.style.display = "flex";
        manualContainerNN.style.display = "flex";

        allButtons.forEach(function(button) {
            button.style.display = "inline-block";
            button.disabled = false;
        });
    }
    else {
        automaticLabel.style.color = "#4682b4";
        automaticLabel.style.fontWeight = "bold";
        automaticContainer.style.display = "flex";

        manualLabel.style.color = "#999";
        manualLabel.style.fontWeight = "normal";
        manualContainer.style.display = "none";
        manualContainerSD.style.display = "none";
        manualContainerNN.style.display = "none";

        allButtons.forEach(function(button) {
            button.style.display = "none";
            button.disabled = true;
        });
    }
}

function updateAddReplaceScanRelayButtons() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var toggleMode = iframeDocument.getElementById("toggleMode");

    var addButtons = iframeDocument.querySelectorAll('.deviceAddButton');
    var replaceButtons = iframeDocument.querySelectorAll('.deviceReplaceButton');
    var scanButtons = iframeDocument.querySelectorAll('.deviceScanButton');
    var relayButtons = iframeDocument.querySelectorAll('.deviceRelayButton');

    var allButtons = [...addButtons, ...replaceButtons, ...scanButtons, ...relayButtons];

    if (toggleMode.checked) {
        allButtons.forEach(function(button) {
            button.style.display = "inline-block";
            button.disabled = false;
        });
    }
    else {
        allButtons.forEach(function(button) {
            button.style.display = "none";
            button.disabled = true;
        });
    }
}

function buildOnLoadMethodAdmin()
{
    var iframe = document.getElementById('mainframe');

    iframe.onload = function() {
        var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

        // Ventana: SUMMARY
        var devicesCounter = iframeDocument.getElementById('devicesCounter');
        var failuresCounter = iframeDocument.getElementById('failuresCounter');

        if (devicesCounter && failuresCounter)
            requestDevicesAndFailuresCount();

        // Ventana: NETWORK
        var totalDevices = iframeDocument.getElementById('totalDevices');
        var totalFailures = iframeDocument.getElementById('totalFailures');

        if (totalDevices && totalFailures)
            requestDevicesAndFailuresCount();

        // TODO: Añadir el resto de componentes de otras ventanas y las consultas en función de la ventana
    };
}

function transformDecimalGroupAddressIntoHexGroupAddress(value) {
    // Ejemplo: Recibe 49153 (la dirección del grupo de emergencia) y devuelve "C001" (la dirección en hexadecimal)
    let hex = value .toString(16);
    hex = hex.toUpperCase();
    return hex;
}

function delGroupPrev() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var selectElem = iframeDocument.getElementById("groupList");
    if(selectElem.options[selectElem.selectedIndex].value == "-") { return; }

    var popup = iframeDocument.getElementById('popupDeletingGroup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var deletingGroupLabel = iframeDocument.getElementById('deletingGroupLabel');
    var deletingGroupButton = iframeDocument.getElementById('deletingGroupButton');
    var closeDelGroupPopup = iframeDocument.getElementById('closeDelGroupPopup');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
    deletingGroupLabel.textContent = "Do you want to delete the group?";
    deletingGroupButton.classList.remove('button-disabled');
    closeDelGroupPopup.style.pointerEvents = "auto";
    closeDelGroupPopup.style.opacity = "1";
}

function editGroupPrev()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var selectElem = iframeDocument.getElementById("groupList");
    if(selectElem.options[selectElem.selectedIndex].value == "-") { return; }

    var popup = iframeDocument.getElementById('popupEditingName');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    var nameInput = iframeDocument.getElementById("newGroupName");
    nameInput.value =  selectElem.options[selectElem.selectedIndex].text;
}

function closeGroupPopup()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popupEdit = iframeDocument.getElementById('popupEditingName');
    var popupDelete = iframeDocument.getElementById('popupDeletingGroup');
    var popupDeleteNode = iframeDocument.getElementById('popupDeletingNode');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');
    
    if(popupEdit) { popupEdit.style.visibility = "hidden"; }
    if(popupDelete) { popupDelete.style.visibility = "hidden"; }
    if(popupDeleteNode) { popupDeleteNode.style.visibility = "hidden"; }
    popupOverlay.style.visibility = "hidden";
}

function delFromGroupPrev()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var selectedNode = iframeDocument.querySelector('#includedNodesList li.selectedDevice');
    var groupList = iframeDocument.getElementById('groupList');
    var groupSelected = groupList.options[groupList.selectedIndex].value;

    if (!selectedNode || groupSelected == '-') { return; }

    var popup = iframeDocument.getElementById('popupDeletingNode');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var deletingNodeLabel = iframeDocument.getElementById('deletingNodeLabel');
    var deletingNodeButton = iframeDocument.getElementById('deletingNodeButton');
    var closeDelNodeFromGroupPopup = iframeDocument.getElementById('closeDelNodeFromGroupPopup');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
    deletingNodeLabel.textContent = "Do you want to delete the node from the group?";
    deletingNodeButton.classList.remove('button-disabled');
    closeDelNodeFromGroupPopup.style.pointerEvents = "auto";
    closeDelNodeFromGroupPopup.style.opacity = "1";
}

function addDevicePrev(value)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popupAddDevice');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    var logAddManual = iframeDocument.getElementById('logAddManual');
    logAddManual.innerHTML = "";

    addDevice(value);
}

function delDevicePrev()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var networkErrorLabel = iframeDocument.getElementById('networkError');
    
    // Seleccionar el nodo marcado en la lista de Network Nodes
    var selectedNode = iframeDocument.querySelector('#networkNodesList li.selectedDevice');
    
    if (selectedNode) {
        networkErrorLabel.style.visibility = "hidden";

        var popup = iframeDocument.getElementById('popupDelDevice');
        var popupOverlay = iframeDocument.getElementById('popupOverlay');
        var deletingDeviceLabel = iframeDocument.getElementById('deletingDeviceLabel');
        var deletingDeviceButton = iframeDocument.getElementById('deletingDeviceButton');
        var closeDelDevPopup = iframeDocument.getElementById('closeDelDevPopup');

        popup.style.visibility = "visible";
        popupOverlay.style.visibility = "visible";
        deletingDeviceLabel.textContent = "Do you want to delete the node from the network?";
        deletingDeviceButton.classList.remove('button-disabled');
        closeDelDevPopup.style.pointerEvents = "auto";
        closeDelDevPopup.style.opacity = "1";

    } else {
        networkErrorLabel.style.visibility = "visible";
        networkErrorLabel.innerText = "No network node selected";
    }
}

function delAllDevicesPrev()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;
    var networkErrorLabel = iframeDocument.getElementById('networkError');

    networkErrorLabel.style.visibility = "hidden";

    var popup = iframeDocument.getElementById('popupDelAllDevices');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var deletingAllDevicesLabel = iframeDocument.getElementById('deletingAllDevicesLabel');
    var deletingAllDevicesButton = iframeDocument.getElementById('deletingAllDevicesButton');
    var closeDelAllPopup = iframeDocument.getElementById('closeDelAllPopup');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
    deletingAllDevicesLabel.textContent = "Do you want to delete ALL the nodes from the network?";
    deletingAllDevicesButton.classList.remove('button-disabled');
    closeDelAllPopup.style.pointerEvents = "auto";
    closeDelAllPopup.style.opacity = "1";
}

function closeWirelessPopup()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
    var popupAdd = iframeDocument.getElementById('popupAddDevice');
    var popupDelete = iframeDocument.getElementById('popupDelDevice');
    var popupDeleteAll = iframeDocument.getElementById('popupDelAllDevices');
    var popupReplacePrev = iframeDocument.getElementById('popupReplaceDevices');
    var popupReplace = iframeDocument.getElementById('popupReplace');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var loader1 = popup.querySelector('.loader');
    loader1.style.animation = "spin 1.5s linear infinite";
    var loader2 = popupAdd.querySelector('.loader');
    loader2.style.animation = "spin 1.5s linear infinite";
    var loader3 = popupReplace.querySelector('.loader');
    loader3.style.animation = "spin 1.5s linear infinite";
    
    if(popup) { popup.style.visibility = "hidden"; }
    if(popupAdd) { popupAdd.style.visibility = "hidden"; }
    if(popupDelete) { popupDelete.style.visibility = "hidden"; }
    if(popupDeleteAll) { popupDeleteAll.style.visibility = "hidden"; }
    if(popupReplacePrev) { popupReplacePrev.style.visibility = "hidden"; }
    if(popupReplace) { popupReplace.style.visibility = "hidden"; }
    popupOverlay.style.visibility = "hidden";
}

function getDefaultGroupsForSelector()
{
    return "<option value='-'> ---- </option>" +
            "<option value='C000'> (Gr0) Lighting </option>" +
            "<option value='C001'> (Gr1) Emergency </option>" +
            "<option value='C002'> (Gr2) Even </option>" +
            "<option value='C003'> (Gr3) Odd </option>";
}

function clearAllDataPrev()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var confirmDeleteData = iframeDocument.getElementById('confirmDeleteData');
    var input = iframeDocument.getElementById('deleteConfirmInput');
    var button = iframeDocument.getElementById('deletingDataButton');
    var closeClearAllPopup = iframeDocument.getElementById('closeClearAllPopup');

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
    confirmDeleteData.textContent = "Do you want to delete ALL the data?";
    input.value = "";
    button.disabled = true;
    closeClearAllPopup.style.pointerEvents = "auto";
    closeClearAllPopup.style.opacity = "1";
}

function closeManageDataPopup()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var input = iframeDocument.getElementById('deleteConfirmInput');
    input.value = "";

    var popup = iframeDocument.getElementById('popup');
    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    
    if(popup) { popup.style.visibility = "hidden"; }
    popupOverlay.style.visibility = "hidden";
}

function changeNodes() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var popup = iframeDocument.getElementById('popupChangeNodes');

    popupOverlay.style.visibility = "visible";
    popup.style.visibility = "visible";

    var positionInput1 = iframeDocument.getElementById('positionInput1');
    var positionInput2 = iframeDocument.getElementById('positionInput2');

    positionInput1.value = "1";
    positionInput2.value = "2";
}

function closeSwap() {
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popupOverlay = iframeDocument.getElementById('popupOverlay');
    var popup = iframeDocument.getElementById('popupChangeNodes');

    popupOverlay.style.visibility = "hidden";
    popup.style.visibility = "hidden";
}

function updateButtonsForNetKeySelected()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var inputAntennaID = iframeDocument.getElementById('antennaID');
    var netKeySelector = iframeDocument.getElementById("netKey");

    // BOTÓN DE EDIT NETKEY
    var editNetKey = iframeDocument.getElementById("editNetKey");

    if(netKeySelector.value == "16") { // si es la Custom NetKey
        editNetKey.disabled = false;
    }
    else { // si es una NetKey por defecto
        editNetKey.disabled = true;
    }

    // BOTÓN DE SAVE CHANGES
    var saveBtn = iframeDocument.getElementById("saveAntennaIDAndNetKey");
    var changed = false;

    if(inputAntennaID.value != antennaID) {
        changed = true;
    }

    if(netKeySelector.value != "16" && netKeySelector.value != netKey) {
        changed = true;
    }
    else if(netKeySelector.value == "16" && (netKeySelector.value != netKey || iframeDocument.getElementById("netKeyByte0").value != "**")) {
        changed = true;
    }
    
    if((inputAntennaID.value < 1 || inputAntennaID.value > 1000) || (netKeySelector.value == "16" && iframeDocument.getElementById("netKeyByte0").value == "")) {
        changed = false; // forzamos desactivado por error de rango de ID o falta de netkey
    }

    if(changed) {
        saveBtn.disabled = false;
    } else {
        saveBtn.disabled = true;
    }
}

function editNetKey()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById("popupEditNetKey");
    var popupOverlay = iframeDocument.getElementById("popupOverlay");

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
}

function clearCustomNetKey()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    for (var i = 0; i < 16; i++) {
        const input = iframeDocument.getElementById(`netKeyByte${i}`);
        if (input) {
            if(netKey == "16")
                input.value = "**";
            else
                input.value = "";
        }
    }

    var popup = iframeDocument.getElementById("popupEditNetKey");
    var popupOverlay = iframeDocument.getElementById("popupOverlay");

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";

    var saveBtn = iframeDocument.getElementById("saveAntennaIDAndNetKey");
    var inputAntennaID = iframeDocument.getElementById('antennaID');
    if(inputAntennaID.value != antennaID) {   
        saveBtn.disabled = false;
    } else {
        saveBtn.disabled = true;
    }

}

function randomizeNetKey()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    for (var i = 0; i < 16; i++) {
        const randByte = Math.floor(Math.random() * 256);
        const hex = randByte.toString(16).toUpperCase().padStart(2, '0');
        const input = iframeDocument.getElementById(`netKeyByte${i}`);
        if (input) input.value = hex;
    }
}

async function copyNetKey()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var newNetKey = "";

    for(var i = 0; i < 16; i++) {
        const input = iframeDocument.getElementById(`netKeyByte${i}`);
        const value = input.value.trim().toUpperCase();
        newNetKey += value;

        if (!/^[0-9A-F]{2}$/.test(value)) {
            alert(`Invalid hex value at byte ${i + 1}: "${value}". Enter two valid hex characters (00 to FF).`);
            return null;
        }
    }

    try {
        if (navigator.clipboard && window.isSecureContext) {
            await navigator.clipboard.writeText(newNetKey);
        } else {
            const ta = document.createElement('textarea');
            ta.value = newNetKey;
            ta.setAttribute('readonly', '');
            ta.style.position = 'fixed';
            ta.style.left = '-9999px';
            document.body.appendChild(ta);
            ta.select();
            ta.setSelectionRange(0, ta.value.length); // iOS
            document.execCommand('copy');
            document.body.removeChild(ta);
        }
        alert('The netKey has been copied');
    } catch (e) {
        alert("The netKey couldn't be copied");
    }
}

function saveCustomNetKey()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    for(var i = 0; i < 16; i++) {
        const input = iframeDocument.getElementById(`netKeyByte${i}`);
        const value = input.value.trim().toUpperCase();

        if (!/^[0-9A-F]{2}$/.test(value)) {
            alert(`Invalid hex value at byte ${i + 1}: "${value}". Enter two valid hex characters (00 to FF).`);
            return null;
        }
    }

    var popup = iframeDocument.getElementById("popupEditNetKey");
    var popupOverlay = iframeDocument.getElementById("popupOverlay");

    popup.style.visibility = "hidden";
    popupOverlay.style.visibility = "hidden";

    var saveBtn = iframeDocument.getElementById("saveAntennaIDAndNetKey");
    saveBtn.disabled = false;
}

function checkAntennaIDChange()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var inputAntennaID = iframeDocument.getElementById('antennaID');
    var netKeySelector = iframeDocument.getElementById("netKey");

    // BOTÓN DE SAVE CHANGES
    var saveBtn = iframeDocument.getElementById("saveAntennaIDAndNetKey");
    var changed = false;

    if(inputAntennaID.value != antennaID) {
        changed = true;
    }

    if(netKeySelector.value != "16" && netKeySelector.value != netKey) {
        changed = true;
    }
    else if(netKeySelector.value == "16" && (netKeySelector.value != netKey || iframeDocument.getElementById("netKeyByte0").value != "**")) {
        changed = true;
    }
    
    if((inputAntennaID.value < 1 || inputAntennaID.value > 1000) || (netKeySelector.value == "16" && iframeDocument.getElementById("netKeyByte0").value == "")) {
        changed = false; // forzamos desactivado por error de rango de ID o falta de netkey
    }

    if(changed) {
        saveBtn.disabled = false;
    } else {
        saveBtn.disabled = true;
    }
}

function showReplacePopup()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById("popupReplaceDevices");
    var popupOverlay = iframeDocument.getElementById("popupOverlay");

    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";

    var scannedSelect = iframeDocument.getElementById("deviceToReplaceSelect");
    scannedSelect.innerHTML = "";

    var scannedDevicesList = iframeDocument.getElementById('scannedDevicesList');
    if(scannedDevicesList) {
        var scannedDevices = scannedDevicesList.getElementsByTagName('li');

        for (var i = 0; i < scannedDevices.length; i++) { 
            var li = scannedDevices[i];
            var span = li.getElementsByTagName('span')[0];
            if (span) {
                var text = span.textContent.trim();
                
                var option = iframeDocument.createElement('option');
                option.value = text;
                option.text = text;

                scannedSelect.appendChild(option);
            }
        }
    }
}

function loadDefaultSettings()
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var ip = iframeDocument.querySelector('input[name="ipValue"]');
    var submask = iframeDocument.querySelector('input[name="submaskValue"]');
    var gateway = iframeDocument.querySelector('input[name="gatewayValue"]');
    var building = iframeDocument.querySelector('input[name="buildingName"]');
    var line = iframeDocument.querySelector('input[name="lineName"]');

    ip.value = "192.168.1.252";
    submask.value = "255.255.255.0";
    gateway.value = "1.1.10.100";
    building.value = "NO_NAME";
    line.value = "NO_NAME";
}