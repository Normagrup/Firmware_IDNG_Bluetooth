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
    var settingsLogs = document.createElement('li');
    var settingsTests = document.createElement('li');
    var settingsUpdateDevice = document.createElement('li');

    var settingsIPConfigLink = document.createElement('a');
    settingsIPConfigLink.onclick = function() { loadPage('s_ipconfig.html'); };
    settingsIPConfigLink.textContent = "IP Configuration";

    var settingsTimeLink = document.createElement('a');
    settingsTimeLink.onclick = function() { loadPage('s_time.html') };
    settingsTimeLink.textContent = "Time";

    var settingsWirelessConfigLink = document.createElement('a');
    settingsWirelessConfigLink.onclick = function() { loadPage('s_wireless.html') };
    settingsWirelessConfigLink.textContent = "Wireless Config";

    var settingsLogsLink = document.createElement('a');
    settingsLogsLink.onclick = function() { loadPage('s_logs.html') };
    settingsLogsLink.textContent = "Log Results";

    var settingsTestsLink = document.createElement('a');
    settingsTestsLink.onclick = function() { loadPage('s_tests.html') };
    settingsTestsLink.textContent = "Emergency Tests";

    var settingsUpdateDeviceLink = document.createElement('a');
    settingsUpdateDeviceLink.onclick = function() { loadPage('s_update.html') };
    settingsUpdateDeviceLink.textContent = "Update Device";

    settingsIPConfig.appendChild(settingsIPConfigLink);
    settingsTime.appendChild(settingsTimeLink);
    settingsWirelessConfig.appendChild(settingsWirelessConfigLink);
    settingsLogs.appendChild(settingsLogsLink);
    settingsTests.appendChild(settingsTestsLink);
    settingsUpdateDevice.appendChild(settingsUpdateDeviceLink);

    settingsButtonMenu.appendChild(settingsIPConfig);
    settingsButtonMenu.appendChild(settingsTime);
    settingsButtonMenu.appendChild(settingsWirelessConfig);
    settingsButtonMenu.appendChild(settingsLogs);
    settingsButtonMenu.appendChild(settingsTests);
    settingsButtonMenu.appendChild(settingsUpdateDevice);

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
    var networkButton = document.createElement('li');
    networkButton.id = "networkButton";
    var networkButtonLink = document.createElement('a');
    networkButtonLink.textContent = "Network";
    networkButtonLink.onclick = function() { loadPage('network.html') };

    var dot = document.createElement('div');
    dot.className = "dot";

    networkButton.appendChild(networkButtonLink);
    networkButton.appendChild(dot);

    var menuContainer = document.getElementById("menuContainer");
    menuContainer.appendChild(networkButton);

    createSettingsButton();
}

function loadAdminMenu()
{
    closeLoginPopup();

    createLogoutButton("admin");

    createAdminMenuButtons();

    loadPage('summary.html');
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

    var scannedDevices = scannedDevicesList.getElementsByTagName('li');
    var networkDevices = networkNodesList.getElementsByTagName('li');

    for (var i = 0; i < scannedDevices.length; i++) { scannedDevices[i].classList.remove('selectedDevice'); }
    for (var i = 0; i < networkDevices.length; i++) { networkDevices[i].classList.remove('selectedDevice'); }

    device.classList.add('selectedDevice');
}

function openGroupControl(buttonText)
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var popup = iframeDocument.getElementById('popup');
	var popupOverlay = iframeDocument.getElementById('popupOverlay');

    var popupText = popup.querySelector('h3');
			
    if (buttonText === 'Lighting') { addressClicked = '49152'; }
    else if (buttonText === 'Emergency') { addressClicked = '49153'; }
    else if (buttonText === 'Even') { addressClicked = '49154'; }
    else if (buttonText === 'Odd') { addressClicked = '49155'; }
    else if (buttonText === 'Group 1') { addressClicked = '49168'; }
    else if (buttonText === 'Group 2') { addressClicked = '49169'; }
    else if (buttonText === 'Group 3') { addressClicked = '49170'; }
    else if (buttonText === 'Group 4') { addressClicked = '49171'; }
    else if (buttonText === 'Group 5') { addressClicked = '49172'; }
    else if (buttonText === 'Group 6') { addressClicked = '49173'; }
    else if (buttonText === 'Group 7') { addressClicked = '49174'; }
    else if (buttonText === 'Group 8') { addressClicked = '49175'; }
    else if (buttonText === 'Group 9') { addressClicked = '49176'; }
    else if (buttonText === 'Group 10') { addressClicked = '49177'; }
    else if (buttonText === 'Group 11') { addressClicked = '49178'; }
    else if (buttonText === 'Group 12') { addressClicked = '49179'; }
    else if (buttonText === 'Group 13') { addressClicked = '49180'; }
    else if (buttonText === 'Group 14') { addressClicked = '49181'; }
    else if (buttonText === 'Group 15') { addressClicked = '49182'; }
    else if (buttonText === 'Group 16') { addressClicked = '49183'; }
    
    popupText.textContent = buttonText;
    
    popup.style.visibility = "visible";
    popupOverlay.style.visibility = "visible";
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
            openNodeControl(this.textContent);
        };
        container.appendChild(button);
        count++;
        if (count >= 100) break;
    }
}

function changeAddress() 
{
    var iframe = document.getElementById('mainframe');
    var iframeDocument = iframe.contentDocument || iframe.contentWindow.document;

    var select = iframeDocument.getElementById('addressList');
    var selectedValue = select.value;
    var range = selectedValue.split('-');
    var start = parseInt(range[0], 10);
    var end = parseInt(range[1], 10);
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
        if (i === 1) { button.textContent = "Lighting"; } 
        else if (i === 2) { button.textContent = "Emergency"; } 
        else if (i === 3) { button.textContent = "Even"; } 
        else if (i === 4) { button.textContent = "Odd"; }
        button.onclick = function() {
            openGroupControl(this.textContent);
        };
        container.appendChild(button);
    }

    for (var j = 1; j <= 16; j++) {
        var groupButton = iframeDocument.createElement('button');
        groupButton.textContent = "Group " + j;
        groupButton.onclick = function() {
            openGroupControl(this.textContent);
        };
        container.appendChild(groupButton);
    }
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

    if (toggleMode.checked) {
        automaticLabel.style.color = "#999";
        automaticLabel.style.fontWeight = "normal";
        automaticContainer.style.display = "none";

        manualLabel.style.color = "#4682b4";
        manualLabel.style.fontWeight = "bold";
        manualContainer.style.display = "flex";
    }
    else {
        automaticLabel.style.color = "#4682b4";
        automaticLabel.style.fontWeight = "bold";
        automaticContainer.style.display = "flex";

        manualLabel.style.color = "#999";
        manualLabel.style.fontWeight = "normal";
        manualContainer.style.display = "none";
    }
}