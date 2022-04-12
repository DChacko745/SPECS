var state = "Go";
var powerStoredState = false;
var powerOutputState = false;
var humidState = false;
var rainState = false;
var tempState = false;
var rainToggleState = false;
var intervalToggleState = false;

// Firebase
var dbRef = firebase.database().ref();

window.addEventListener('load', onLoad);
function onLoad(event) {
  initSensor();
  initButton();
  initExtras();
}
function initSensor() {
  dbRef.child("Users").child("test").child("deviceMacAddress").child("sensorData").child("humidity").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('humid').innerHTML = data.toString() + " %";
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("sensorData").child("powerGenerated").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('powerOutput').innerHTML = data.toString() + " Volts";
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("sensorData").child("systemPowerDraw").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('powerStored').innerHTML = data.toString() + " Watts";
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("sensorData").child("rain").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('rain').innerHTML = data.toString() + " %";
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("sensorData").child("temperature").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('temp').innerHTML = data.toString() + " &deg;F";
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastClean").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('lastCleaning').innerHTML = data.toString();
    document.getElementById('lastCleaningInt').innerHTML = data.toString();
  });
}
function initButton() {
  document.getElementById('button').addEventListener('click', toggle);
  document.getElementById('intervalSet').addEventListener('click', toggle);
  document.getElementById('refresh').addEventListener('click', toggle);
  document.getElementById('powerStoredInfo').addEventListener('click', toggle);
  document.getElementById('powerOutputInfo').addEventListener('click', toggle);
  document.getElementById('humidInfo').addEventListener('click', toggle);
  document.getElementById('rainInfo').addEventListener('click', toggle);
  document.getElementById('tempInfo').addEventListener('click', toggle);
  document.getElementById('rainToggle').addEventListener('click', toggle);
  document.getElementById('intervalToggle').addEventListener('click', toggle);
}
function initExtras() {
  // initializing anything else from database that isn't sensor data
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastClean").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('lastCleaning').innerHTML = data.toString();
    document.getElementById('lastCleaningInt').innerHTML = data.toString();
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("nextClean").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('nextCleaning').innerHTML = data.toString();
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastSensorRead").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('refreshTime').innerHTML = data.toString();
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("cleaningInterval").get().then((snapshot) => {
    const data = snapshot.val();
    var numMin = parseInt(data/60)%60;
    var numHr = parseInt((data/60-numMin)/60)%24;
    var numDay = parseInt(((data/60-numMin)/60-numHr)/24);

    document.getElementById('Minutes').getElementsByTagName('option')[numMin/5+1].selected = true;
    document.getElementById('Hours').getElementsByTagName('option')[numHr+1].selected = true;
    document.getElementById('Days').getElementsByTagName('option')[numDay+1].selected = true;
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").child("toggleAutoClean").get().then((snapshot) => {
    const data = snapshot.val();
    if (data == true) {
      rainToggleState = true;
      document.getElementById('rainToggleImg').src = 'scripts/images/toggleOn.png';
      document.getElementById('rainTog').innerHTML = 'On';
    }
    else {
      rainToggleState = false;
      document.getElementById('rainToggleImg').src = 'scripts/images/toggleOff.png';
      document.getElementById('rainTog').innerHTML = 'Off';
    }
  });
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").child("toggleIntervalCleaning").get().then((snapshot) => {
    const data = snapshot.val();
    if (data == true) {
      intervalToggleState = true;
      document.getElementById('intervalTogImg').src = 'scripts/images/toggleOn.png';
      document.getElementById('intervalTog').innerHTML = 'On';
    }
    else {
      intervalToggleState = false;
      document.getElementById('intervalTogImg').src = 'scripts/images/toggleOff.png';
      document.getElementById('intervalTog').innerHTML = 'Off';

      document.getElementById('Days').style.visibility = 'hidden';
      document.getElementById('Hours').style.visibility = 'hidden';
      document.getElementById('Minutes').style.visibility = 'hidden';
      document.getElementById('intervalSet').style.visibility = 'hidden';
      document.getElementById('nextCleaningH').style.visibility = 'hidden';
      document.getElementById('nextCleaning').style.visibility = 'hidden';
      document.getElementById('lastCleaningH').style.visibility = 'hidden';
      document.getElementById('lastCleaningInt').style.visibility = 'hidden';
      document.getElementById('Days').style.display = 'none';
      document.getElementById('Hours').style.display = 'none';
      document.getElementById('Minutes').style.display = 'none';
      document.getElementById('intervalSet').style.display = 'none';
      document.getElementById('nextCleaningH').style.display = 'none';
      document.getElementById('nextCleaning').style.display = 'none';
      document.getElementById('lastCleaningH').style.display = 'none';
      document.getElementById('lastCleaningInt').style.display = 'none';
      document.getElementById('fakeLoad').style.display = 'none';
      document.getElementById('refreshInterval').style.display = 'none';
    }
  });
}

dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("nextClean").on('value', function(snapshot) {
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("nextClean").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('nextCleaning').innerHTML = data.toString();
    document.getElementById('refreshInterval').style.visibility = 'hidden';
    document.getElementById('intervalSet').style.background = '#0f8b8d';
    document.getElementById('intervalSet').disabled = false;
  });
});
dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastClean").on('value', function(snapshot) {
  dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastClean").get().then((snapshot) => {
    const data = snapshot.val();
    document.getElementById('lastCleaning').innerHTML = data.toString();
    document.getElementById('lastCleaningInt').innerHTML = data.toString();
  });
});
dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").child("isRefreshing").on('value', function(snapshot) {
  if (snapshot.val() == false) {
    initSensor();
    document.getElementById('refresh').disabled = false;
    document.getElementById('refreshImg').src = 'scripts/images/refresh.png';
    dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastSensorRead").get().then((snapshot) => {
      const data = snapshot.val();
      document.getElementById('refreshTime').innerHTML = data.toString();
    });
  }
});
dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").child("isUpdatingInterval").on('value', function(snapshot) {
  if (snapshot.val() == false) {
    document.getElementById('refreshInterval').style.visibility = 'hidden';
    document.getElementById('intervalSet').style.background = '#0f8b8d';
    document.getElementById('intervalSet').disabled = false;
  }
});
function toggle(){
  if (this.value == 'button') {
    state = "Cleaning...";
    document.getElementById('state').innerHTML = state;

    this.style.background = '#8c8c8c';
    var id = this.id;
    // Disables button so you can't press it again
    this.disabled=true;

    // Loading Gif
    document.getElementById('loading').style.visibility = 'visible';
    document.getElementById('loading').style.width = '500px';
    document.getElementById('loading').style.height = '50px';

    dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'isCleaning':true});

    dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").child("isCleaning").on('value', function(snapshot) {
      if (snapshot.val() == false) {
        state = "Go";
        document.getElementById('state').innerHTML = state;

        document.getElementById(id).style.background = '#0f8b8d';
        // Re enables button to press again
        document.getElementById(id).disabled = false;

        // Loading Gif
        document.getElementById('loading').style.visibility = 'hidden';
        document.getElementById('loading').style.width = '0px';
        document.getElementById('loading').style.height = '0px';
      }
    });
  }
  else if (this.value == 'powerStored') {
    if (!powerStoredState) {
      powerStoredState = true;
        document.getElementById('powerStoredDesc').style.visibility = 'visible';
        document.getElementById('powerStoredDesc').style.fontSize = 'medium';
    }
    else {
      powerStoredState = false;
      document.getElementById('powerStoredDesc').style.visibility = 'hidden';
      document.getElementById('powerStoredDesc').style.fontSize = 'xx-small';
    }
  }
  else if (this.value == 'powerOutput') {
    if (!powerOutputState) {
      powerOutputState = true;
        document.getElementById('powerOutputDesc').style.visibility = 'visible';
        document.getElementById('powerOutputDesc').style.fontSize = 'medium';
    }
    else {
      powerOutputState = false;
      document.getElementById('powerOutputDesc').style.visibility = 'hidden';
      document.getElementById('powerOutputDesc').style.fontSize = 'xx-small';
    }
  }
  else if (this.value == 'humid') {
    if (!humidState) {
        humidState = true;
        document.getElementById('humidDesc').style.visibility = 'visible';
        document.getElementById('humidDesc').style.fontSize = 'medium';
    }
    else {
      humidState = false;
      document.getElementById('humidDesc').style.visibility = 'hidden';
      document.getElementById('humidDesc').style.fontSize = 'xx-small';
    }
  }
  else if (this.value == 'rain') {
    if (!rainState) {
        rainState = true;
        document.getElementById('rainDesc').style.visibility = 'visible';
        document.getElementById('rainDesc').style.fontSize = 'medium';
    }
    else {
      rainState = false;
      document.getElementById('rainDesc').style.visibility = 'hidden';
      document.getElementById('rainDesc').style.fontSize = 'xx-small';
    }
  }
  else if (this.value == 'temp') {
    if (!tempState) {
        tempState = true;
        document.getElementById('tempDesc').style.visibility = 'visible';
        document.getElementById('tempDesc').style.fontSize = 'medium';
    }
    else {
      tempState = false;
      document.getElementById('tempDesc').style.visibility = 'hidden';
      document.getElementById('tempDesc').style.fontSize = 'xx-small';
    }
  }
  else if (this.value == 'interval') {
    // setting interval and checking for bad values
    var day = document.getElementById('Days').value;
    var hour = document.getElementById('Hours').value;
    var min = document.getElementById('Minutes').value;
    if ((day == "0" && hour == "0" && min == "0") || day == "Days" || hour == "Hours" || min == "Minutes") {
      alert("Please enter valid time interval");
    }
    else {
      var numDay = parseInt(day);
      var numHr = parseInt(hour);
      var numMin = parseInt(min);
      // convert to secs for database
      var secs = 60*(60*((24*numDay)+numHr)+numMin);
      dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").update({'cleaningInterval':secs});
      dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'isUpdatingInterval':true});
      document.getElementById('refreshInterval').style.visibility = 'visible';
      this.disabled = true;
      this.style.background = '#8c8c8c';
      var id = this.id;
    }
  }
  else if (this.value == 'refresh') {
    // retrieve the respective data from firebase
    dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'isRefreshing':true});
    this.disabled=true;
    document.getElementById('refreshImg').src = 'scripts/images/refreshLoad.gif';
    var id = this.id;
    document.getElementById('humid').innerHTML = '...';
    document.getElementById('powerOutput').innerHTML = '...';
    document.getElementById('powerStored').innerHTML = '...';
    document.getElementById('rain').innerHTML = '...';
    document.getElementById('temp').innerHTML = '...';
    document.getElementById('refreshTime').innerHTML = '...';
    dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").child("isRefreshing").on('value', function(snapshot) {
      if (snapshot.val() == false) {
        initSensor();
        document.getElementById(id).disabled = false;
        document.getElementById('refreshImg').src = 'scripts/images/refresh.png';
        dbRef.child("Users").child("test").child("deviceMacAddress").child("systemData").child("lastSensorRead").get().then((snapshot) => {
          const data = snapshot.val();
          document.getElementById('refreshTime').innerHTML = data.toString();
        });
      }
    });
  }
  else if (this.value == 'rainToggle') {
    if (!rainToggleState) {
      // auto-clean toggle on, update in firebase
      dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'toggleAutoClean':true});

      rainToggleState = true;
      document.getElementById('rainToggleImg').src = 'scripts/images/toggleOn.png';
      document.getElementById('rainTog').innerHTML = 'On';
    }
    else {
      // auto-clean toggle off, update in firebase
      dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'toggleAutoClean':false});

      rainToggleState = false;
      document.getElementById('rainToggleImg').src = 'scripts/images/toggleOff.png';
      document.getElementById('rainTog').innerHTML = 'Off';
    }
  }
  else if (this.value == 'intervalToggle') {
    if (!intervalToggleState) {
      // auto-clean toggle on, update in firebase
      dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'toggleIntervalCleaning':true});

      intervalToggleState = true;
      document.getElementById('intervalTogImg').src = 'scripts/images/toggleOn.png';
      document.getElementById('intervalTog').innerHTML = 'On';

      document.getElementById('Days').style.visibility = 'visible';
      document.getElementById('Hours').style.visibility = 'visible';
      document.getElementById('Minutes').style.visibility = 'visible';
      document.getElementById('intervalSet').style.visibility = 'visible';
      document.getElementById('nextCleaningH').style.visibility = 'visible';
      document.getElementById('nextCleaning').style.visibility = 'visible';
      document.getElementById('lastCleaningH').style.visibility = 'visible';
      document.getElementById('lastCleaningInt').style.visibility = 'visible';
      document.getElementById('Days').style.display = 'inline';
      document.getElementById('Hours').style.display = 'inline';
      document.getElementById('Minutes').style.display = 'inline';
      document.getElementById('intervalSet').style.display = 'inline';
      document.getElementById('nextCleaningH').style.display = 'inline';
      document.getElementById('nextCleaning').style.display = 'inline';
      document.getElementById('lastCleaningH').style.display = 'block';
      document.getElementById('lastCleaningInt').style.display = 'inline';
      document.getElementById('fakeLoad').style.display = 'inline-block';
      document.getElementById('refreshInterval').style.display = 'inline-block';
    }
    else {
      // interval toggle off, update in firebase
      dbRef.child("Users").child("test").child("deviceMacAddress").child("systemSettings").update({'toggleIntervalCleaning':false});

      intervalToggleState = false;
      document.getElementById('intervalTogImg').src = 'scripts/images/toggleOff.png';
      document.getElementById('intervalTog').innerHTML = 'Off';
      
      document.getElementById('Days').style.visibility = 'hidden';
      document.getElementById('Hours').style.visibility = 'hidden';
      document.getElementById('Minutes').style.visibility = 'hidden';
      document.getElementById('intervalSet').style.visibility = 'hidden';
      document.getElementById('nextCleaningH').style.visibility = 'hidden';
      document.getElementById('nextCleaning').style.visibility = 'hidden';
      document.getElementById('lastCleaningH').style.visibility = 'hidden';
      document.getElementById('lastCleaningInt').style.visibility = 'hidden';
      document.getElementById('Days').style.display = 'none';
      document.getElementById('Hours').style.display = 'none';
      document.getElementById('Minutes').style.display = 'none';
      document.getElementById('intervalSet').style.display = 'none';
      document.getElementById('nextCleaningH').style.display = 'none';
      document.getElementById('nextCleaning').style.display = 'none';
      document.getElementById('lastCleaningH').style.display = 'none';
      document.getElementById('lastCleaningInt').style.display = 'none';
      document.getElementById('fakeLoad').style.display = 'none';
      document.getElementById('refreshInterval').style.display = 'none';
    }
  }
}