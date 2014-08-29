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

var myPoint;

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

var user = {
    symbol: null,
    x: null,
    y: null,
    positionKnown: false,
    uniqueId: 1000*Math.random()
}


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

//---- ADD LAYERS ----//
var layerUser = new OpenLayers.Layer.Vector('vector');
map.addLayer(layerUser);



//---- KEEP TRACK OF WHERE THE USER IS ----//
findMe.events.register("locationupdated",findMe,function(e) {

    //alert("I found you!");
    user.x = e.point.x;
    user.y = e.point.y;

    console.log("gotcha");

    if (!user.positionKnown) {

        console.log("locking onto position")
        user.positionKnown = true;
        map.setCenter([user.x, user.y], map.getZoom());
        zoomGradual(15);
        requestKml();
    }

    layerUser.destroyFeatures(user.symbol);

    user.symbol = new OpenLayers.Feature.Vector(
            OpenLayers.Geometry.Polygon.createRegularPolygon(
                new OpenLayers.Geometry.Point(e.point.x, e.point.y),
                e.position.coords.accuracy/2,
                40,
                0
            ),
            {},
            {
                fillColor: '#FF0000',
                fillOpacity: 1,
                strokeWidth: 0
            }
        );
        layerUser.addFeatures(user.symbol);

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

        zoomCounter+=2;
        if (zoomCounter==Math.abs(level-start)) {
            map.setCenter([user.x,user.y],level);
            window.clearInterval(intervalId);
        }

    }, 1000);

}

function requestKml()
{
    var xmlhttp = (window.XMLHttpRequest) ? new XMLHttpRequest() : new activeXObject("Microsoft.XMLHTTP");

    myPoint = new OpenLayers.Geometry.Point(user.x, user.y).transform(toProjection, fromProjection);

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
}