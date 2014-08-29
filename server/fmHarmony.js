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

var gui = new dat.GUI({ autoPlace: false, width: 125});
var customContainer = document.getElementById('gui');
customContainer.appendChild(gui.domElement);

gui.closed = true;

//---- ADD STUFF TO THE GUI ----//
var guiOptions = {

    Jazz: function() { 
        alert("you want to listen to Jazz!");
    },

    Country: function() {
        alert("You're pretty good at drinkin' bear");
    },

    Classical: function() {
        alert("you want to listen to Classical.... boooo");
    },

    Rock: function() {
        alert("Rock and Roll, Baby")
    },

    HipHop: function() {
        alert("Hippity Hoppity");
    },

    'R&B': function() {
        alert("hey lil' mama lemme whisper in ya ear");
    },

    Blues: function() {
        alert("Slappa da bass");
    }

};

gui.add(guiOptions,'Jazz');
gui.add(guiOptions,'Country');
gui.add(guiOptions,'Classical');
gui.add(guiOptions,'Rock');
gui.add(guiOptions,'HipHop');
gui.add(guiOptions,"R&B");
gui.add(guiOptions,'Blues');



//---- GLOBAL VARS ----//
var fromProjection = new OpenLayers.Projection("EPSG:4326"); // transform from WGS 1984
var toProjection = new OpenLayers.Projection("EPSG:900913"); // to Spherical Mercator Projection
var host = "./KML/";
var popup = null;
var popupFeature = null;
var kmlRefreshInterval = 30000;

var user = {
    symbol: null,
    x: null,
    y: null,
    positionKnown: false,
    uniqueId: 1000*Math.random()
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

    user.x = e.point.x;
    user.y = e.point.y;

    console.log("gotcha");

    if (!user.positionKnown) {

        console.log("locking onto position")
        user.positionKnown = true;
        map.setCenter([user.x, user.y], map.getZoom());
        zoomGradual(10);
        requestKml();
    } else {
        console.log("erasing previous user.symbol");
        layerUser.destroyFeatures(user.symbol);
    }

    user.symbol = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point(e.point.x, e.point.y));

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

    var request = "httpHandler.php?lat="+myPoint.y+"&lon="+myPoint.x+"&uniqueId="+user.uniqueId;

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
    setTimeout(loadKml, 5000);
}

function loadKml()
{
    console.log("Get after it: "+kmlUrl);

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

    if ((map.getLayersByName("Stations"))[0]==null)
    {
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
    }
    else {  
            //refresh the current layer with the new KML data

            console.log("refreshing existing KML layer");

            //setting loaded to false unloads the layer//
            kmlLayer.loaded = false;

            //setting visibility to true forces a reload of the layer//
            kmlLayer.setVisibility(true);

            //the refresh will force it to get the new KML data//
            kmlLayer.refresh({ force: true, params: { 'key': Math.random()} });
    }

    //ask for a kml refresh after a reasonable amount of time
    setTimeout(requestKml, kmlRefreshInterval);
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
        popup.backgroundColor = "rgb(153,204,255)";
    else if (feature.style.externalGraphic=="./icons/station2.png")
        popup.backgroundColor = "rgb(204,255,204)";
    else if (feature.style.externalGraphic=="./icons/station3.png")
        popup.backgroundColor = "rgb(255,255,204)";
    else if (feature.style.externalGraphic=="./icons/station4.png")
        popup.backgroundColor = "rgb(255,178,102)";
    else if (feature.style.externalGraphic=="./icons/station5.png")
        popup.backgroundColor = "rgb(255,51,51)";

  popup.opacity = 0.9;
  map.addPopup(popup);
  return popup;
}