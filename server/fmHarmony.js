/*
#=================================================================
# APPLICATION NAME: fmHarmony.js
#
#
# DESCRIPTION: The In-Browser Graphical User Interface for the fmHarmony Software Package
#
# ENVIRONMENT PARAMS:
#
# INPUT PARAMETERS: 
#
# OUTPUT PARAMETERS:
#
# AUTHORS: Kyle Nolan, Marcel Marki
#
# DATE: 2013.08.08
#
# CONTRIBUTING AUTHORS:
#
# REVISIONS: 
# 
#
#  
#================================================================
*/

//---- CREATE THE GUI ----//

var gui = new dat.GUI({ autoPlace: false, width: 150});
var customContainer = document.getElementById('gui');
customContainer.appendChild(gui.domElement);

gui.closed = true;

//---- ADD STUFF TO THE GUI ----//
var guiOptions = {

    Jazz: function() { 
        findBestStation("jazz");
    },

    Country: function() {
        findBestStation("country");
    },

    Classical: function() {
        findBestStation("classical");
    },

    Club: function() {
        findBestStation("club");
    },

    Rock: function() {
        findBestStation("rock");
    },

    'Hip-Hop': function() {
        findBestStation("hip-hop");
    },

    'R&B': function() {
        findBestStation("r&b");
    },

    Oldies: function() {
        findBestStation("oldies");
    },

    Christian: function() {
        findBestStation("christian");
    },

    Alternative: function() {
        findBestStation("alternative");
    },

    Pop: function() {
        findBestStation("pop");
    },

    Talk: function() {
        findBestStation("talk");
    },

    Foreign: function() {
        findBestStation("foreign");
    },

    Sync: function() {
        if (kmlRequestTimer != null)
        {
            clearInterval(kmlRequestTimer);
            kmlRequestTimer = null;
        }
        requestKml();
        
        document.getElementById('processing').style.opacity = processingOpacity;
    },

    Radius: 50,

};

var folderGenres = gui.addFolder('Select Genre');
var folderSettings = gui.addFolder('Settings');

folderGenres.add(guiOptions,'Alternative');
folderGenres.add(guiOptions,'Christian');
folderGenres.add(guiOptions,'Classical');
folderGenres.add(guiOptions,'Club');
folderGenres.add(guiOptions,'Country');
folderGenres.add(guiOptions,'Foreign');
folderGenres.add(guiOptions,'Hip-Hop');
folderGenres.add(guiOptions,'Jazz');
folderGenres.add(guiOptions,'Oldies');
folderGenres.add(guiOptions,'Pop');
folderGenres.add(guiOptions,'Rock');
folderGenres.add(guiOptions,"R&B");
folderGenres.add(guiOptions,'Talk');

folderSettings.add(guiOptions, 'Radius',5, 150);
folderSettings.add(guiOptions, 'Sync');

//---- GLOBAL VARS ----//
var fromProjection = new OpenLayers.Projection("EPSG:4326"); // transform from WGS 1984
var toProjection = new OpenLayers.Projection("EPSG:900913"); // to Spherical Mercator Projection
var host = "./KML/";
var popup = null;
var popupFeature = null;
var kmlRefreshInterval = 30000;
var kmlRefreshTimer = null;
var kmlRequestTimer = null;
var processingOpacity = 0.6;
var stationsWindow = null;

var stationFeatures = [];

var user = {
    symbol: null,
    x: null,
    y: null,
    positionKnown: false,
    uniqueId: 1000*Math.random(),
    pressedMove: false
}

var kmlUrl = host+user.uniqueId+".kml?key="+Math.random();

// ----- CUSTOM MAP CONTROLS ----- //
var findMe = new OpenLayers.Control.Geolocate({
    bind: false,
    watch: true,
    geolocationOptions: {
        enableHighAccuracy: false,
        maximumAge: 0,
        timeout: 10000
    }
});

var mapScale = new OpenLayers.Control.ScaleLine({
                minWidth: 75,
                maxWidth: 350
            });


// ------ CREATE THE MAP ------ //
var map = new OpenLayers.Map({
    div: "map",
    transitionEffect: 'resize',
    //zoomMethod: null,
    //define map layers
    layers: [
        new OpenLayers.Layer.OSM("OSM (with buffer)", null, {buffer: 2})
    ],
    //define map controls
    controls: [
        new OpenLayers.Control.Navigation({
            dragPanOptions: {
                enableKinetic: true
            }
        }),
        findMe,
        //new OpenLayers.Control.PanZoomBar(),
        mapScale,
        //new OpenLayers.Control.LayerSwitcher(),
        new OpenLayers.Control.KeyboardDefaults(),
        new OpenLayers.Control.Attribution()
    ]
});

//---- ACTIVATE CONTROLS ----//
findMe.activate();


//---- SET UP INITIAL MAP ----//
var zoomTo = 3;
//-69.064,44.210
map.setCenter(new OpenLayers.LonLat(-100,40).transform(fromProjection,toProjection),zoomTo);


//---- DEFINE STYLES ----//
var styleMap = new OpenLayers.StyleMap({
    'pointRadius': 60,
    'externalGraphic': './icons/pin.png'
});


//---- ADD LAYERS ----//
var kmlLayer = null;

var layerUser = new OpenLayers.Layer.Vector("userVector", {styleMap: styleMap});
map.addLayer(layerUser);


//---- CONFIGURE FEATURE SELECTION ----//
var selectControl = null;

//---- KEEP TRACK OF WHERE THE USER IS ----//
findMe.events.register("locationupdated",findMe,function(e) {

    //take position updates until you forcibly move around
    if (user.pressedMove==false)
    {
        user.x = e.point.x;
        user.y = e.point.y;
        
        if (user.symbol!=null)
            layerUser.destroyFeatures(user.symbol);

        user.symbol = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point(e.point.x, e.point.y));
    }

    console.log("gotcha");

    if (!user.positionKnown) {

        console.log("locking onto position")
        user.positionKnown = true;
        map.setCenter([user.x, user.y], map.getZoom());
        zoomGradual(10);
        if (kmlRequestTimer != null)
        {
            clearInterval(kmlRequestTimer);
            kmlRequestTimer = null;
        }
        requestKml();
        

    } else {
        console.log("erasing previous user.symbol");
        layerUser.destroyFeatures(user.symbol);
    }

    /*user.symbol.attributes = user.symbol.attributes || {};
    user.symbol.attributes.style = OpenLayers.Util.extend(OpenLayers.Feature.Vector.style['default'], {
        'externalGraphic': './icons/arrow.png',
        'graphicWidth': 48,
        'graphicHeight': 48
    })*/

        console.log("adding new user.symbol");
        layerUser.addFeatures([ user.symbol ]);

});

findMe.events.register("locationfailed",this,function() {
    OpenLayers.Console.log('Location detection failed');
});


//---- CUSTOM FUNCTIONS ----//

function zoomGradual(level)
{
    var zoomCounter = 0;
    var start = map.getZoom();
    var iterations = Math.abs(level-start);

    var intervalId = window.setInterval(function() {

        map.setCenter([user.x,user.y],start+2*zoomCounter);

        if (level > start)
            {map.zoomIn(); map.zoomIn();}
        else
            {map.zoomOut(); map.zoomOut();}

        zoomCounter++;
        if (zoomCounter*2>=Math.abs(level-start)) {
            map.setCenter([user.x,user.y],level);
            window.clearInterval(intervalId);
        }

    }, 1000);

}

function requestKml()
{
    var xmlhttp = (window.XMLHttpRequest) ? new XMLHttpRequest() : new activeXObject("Microsoft.XMLHTTP");

    var myPoint = new OpenLayers.Geometry.Point(user.x, user.y).transform(toProjection, fromProjection);

    var request = "httpHandler.php?lat="+myPoint.y+"&lon="+myPoint.x+"&rad="+guiOptions.Radius+"&uniqueId="+user.uniqueId;

    console.log("\nKML Please!\t\t"+request+"\n");

    xmlhttp.open( 'GET', request, true );

    xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

    xmlhttp.onreadystatechange=function(){
    if(xmlhttp.readyState==4){
        if(xmlhttp.status==200){
            console.log(xmlhttp.responseText);
            }
        }
    }
    xmlhttp.send(null);

    //load in the kml file after a reasonable amount of time
    if (kmlRefreshTimer != null)
        {
            clearInterval(kmlRefreshTimer);
            kmlRefreshTimer = null;
        }
    kmlRefreshTimer = setTimeout(loadKml, 5000);
}

function loadKml()
{
    console.log("Get after it: "+kmlUrl);

    //give up knowledge of all station features as new ones come in
    stationFeatures = [];

    if ((map.getLayersByName("Stations"))[0]==null)
    {
            kmlLayer = new OpenLayers.Layer.Vector("Stations", {
                projection: fromProjection,
                strategies: [new OpenLayers.Strategy.Fixed()],
                protocol: new OpenLayers.Protocol.HTTP({
                    //set the url to our own var
                    url: kmlUrl,
                    //format this layer as KML//
                    format: new OpenLayers.Format.KML({
                        //maxDepth is how deep it will follow network links
                        maxDepth: 2,
                        extractStyles: true,
                        extractAttributes: true
                    })
                })
            });

        console.log("adding first KML layer");
        map.addLayer(kmlLayer);

        //create select control
        selectControl = new OpenLayers.Control.SelectFeature(
            [kmlLayer, layerUser], {
                multiple: false, 
                hover: false
            }
        );

        //activate select control
        map.addControl(selectControl);
        selectControl.activate();

        //allow kmlLayer select feature event handling
        kmlLayer.events.on({
                    "featureselected": function(e) {
                        //alert("test");
                        onFeatureSelect(e.feature);
                    }
                });

        //processing to occur directly after a feature is inserted into the KML vector layer
        kmlLayer.onFeatureInsert = function(feature)
        {
            //keep track of each feature as it comes in
            stationFeatures.push(feature);
        }
    }
    else {  
            //refresh the current layer with the new KML data
            kmlLayer.url = kmlUrl;

            console.log("refreshing existing KML layer");

            //setting loaded to false unloads the layer//
            kmlLayer.loaded = false;

            //setting visibility to true forces a reload of the layer//
            kmlLayer.setVisibility(true);

            //the refresh will force it to get the new KML data//
            kmlLayer.refresh({ force: true, params: { 'key': Math.random()} });
    }

    //ask for a kml refresh after a reasonable amount of time
    if (kmlRequestTimer != null)
        {
            clearInterval(kmlRequestTimer);
            kmlRequestTimer = null;
        }

    kmlRequestTimer = setTimeout(requestKml, kmlRefreshInterval);
    document.getElementById('processing').style.opacity = 0;
}

//what to do when you select a KML vector layer feature
function onFeatureSelect(feature)
{
  generatePopup(feature, false);
}


//generate a popup for this feature
function generatePopup(feature, keepInView)
{
    popupFeature = feature;
    if (popup != null)
    {
        map.removePopup(popup);
        popup.destroy();
        delete popup;
        popup=null;
    }

  popup = new OpenLayers.Popup(
  "chicken",
  new OpenLayers.LonLat(feature.geometry.x,feature.geometry.y),
  null,
  '<b>'+feature.attributes.name+'</b><p>'+feature.attributes.description+'</p>',
  null,
  true
  );
  popup.autoSize = true;
  popup.panMapIfOutOfView = keepInView;
  popup.addCloseBox(function() {map.removePopup(popup); popup.destroy(); delete popup; popup=null;});

    if (feature.style.externalGraphic=="./icons/station1.png")
        popup.backgroundColor = "rgb(153,255,153)";
    else if (feature.style.externalGraphic=="./icons/station2.png")
        popup.backgroundColor = "rgb(204,255,103)";
    else if (feature.style.externalGraphic=="./icons/station3.png")
        popup.backgroundColor = "rgb(255,204,153)";
    else if (feature.style.externalGraphic=="./icons/station4.png")
        popup.backgroundColor = "rgb(255,153,153)";
    else if (feature.style.externalGraphic=="./icons/station5.png")
        popup.backgroundColor = "rgb(255,51,51)";

  popup.opacity = 0.9;
  map.addPopup(popup);
  return popup;
}

function findBestStation(genre)
{
    //bail out if we don't know about any stations yet
    if (stationFeatures.length < 1)
    {
        console.log("stations? I don't even know what a station is.");
        return;
    }

    var matches = [];
    //traverse all known stations and look for matching genre
    for (var i=0; i<stationFeatures.length; i++)
    {
        if (stationFeatures[i].attributes.description.indexOf(genre) > 0)
            matches.push(stationFeatures[i]);
    }

    if (matches.length < 1)
    {
        console.log("No "+genre+" stations in your area");
        return;
    } 
    else
    {
        //traverse all matches and identify the best one
        var distance = null;
        var distance2 = null;
        var closestStation = null;
        var nextClosestStation = null;
        var theStation = null;

        for (var i=0; i<matches.length; i++)
        {
            var testDist = Math.sqrt(Math.pow((user.x - matches[i].geometry.x),2)+ Math.pow((user.y - matches[i].geometry.y),2));
            if ((testDist < distance)||(distance==null))
            {
                closestStation = matches[i];
                distance = testDist;
            } else if ((testDist < distance2)||(distance2==null))
            {
                nextClosestStation = matches[i];
                distance2 = testDist;
            }
        }

        if (closestStation!=null)
        {
            if (popup != null)
            {
                //console.log("there is a popup about!");
                //if the active popup is already for this station, pick the next closest match (if there is one)
                if ((closestStation == popupFeature)&&(nextClosestStation!=null))
                {
                    theStation = nextClosestStation;
                    //console.log("using next closest station!");
                } else theStation = closestStation;
            } else theStation = closestStation;

            //generate popup and pan to the appropriate station
            generatePopup(theStation,false);
            map.panTo(new OpenLayers.LonLat(theStation.geometry.x,theStation.geometry.y));

        } else {
            console.log("ERROR: distance algorithm");
        }

    }
}

function moveUser()
{
    user.pressedMove = true;
    clearTimeout(kmlRefreshTimer);
    layerUser.destroyFeatures(user.symbol);

    user.positionKnown = true;

    user.x = map.getCenter().lon;
    user.y = map.getCenter().lat;

    user.symbol = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point(user.x, user.y));
    layerUser.addFeatures(user.symbol);

    if (kmlRequestTimer != null)
        {
            clearInterval(kmlRequestTimer);
            kmlRequestTimer = null;
        }

    requestKml();
    document.getElementById('processing').style.opacity = processingOpacity;
}

function logStations()
{
    if (stationsWindow != null)
    {
        stationsWindow.close();
        stationsWindow = null;
    }

    stationsWindow = window.open('','Stations Log',"width=400, height = 40");
    //stationsWindow.document.write('Stations within '+guiOptions.radius+' miles of '+user.x+', '+user.y+':\n\n');
    for (var i=0; i<stationFeatures.length; i++)
    {
        var feature = stationFeatures[i];
        var lat, lon, erp, haat, callsign, freq;

        var descr = feature.attributes.description;
        var title = feature.attributes.name;

        erp = descr.substring(descr.indexOf("ERP: ")+5, descr.indexOf("(kW)"));
        haat = descr.substring(descr.indexOf("HAAT: ")+6, descr.indexOf("(m)"));
        callsign = title.substring(title.indexOf(">")+1, title.indexOf(" - "));
        freq = title.substring(title.indexOf(" - ")+3);

        var point = new OpenLayers.LonLat(feature.geometry.x, feature.geometry.y);
        point.transform(toProjection, fromProjection);
        lat = point.lat;
        lon = point.lon;

        stationsWindow.document.write("call: "+callsign+" freq: "+freq+" lat: "+lat.toFixed(5)+" lon: "+lon.toFixed(5)+" erp: "+erp+" haat: "+haat+"<br>");

    }
    stationsWindow.focus();
}