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

    Grid: function() {
        if (gridLayer!=null)
        {
            //console.log('destroying grid');
            destroyGridLayer();
        } else {
            //console.log('creating grid');
            createGridLayer();
        }
    },

    Log: function() {
        logStations();
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
folderSettings.add(guiOptions, 'Grid');
folderSettings.add(guiOptions, 'Log');

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
var gridSideLength = 10;
var gridOrigin = new OpenLayers.LonLat(-71.121,47.487);
var gridWidth = -66.924 - -1*71.121;
var gridHeight = -43.033 - -1*47.487;

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
        new OpenLayers.Control.LayerSwitcher(),
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

var styleMapText = new OpenLayers.StyleMap({
    'pointRadius': 0,
        label : "${labelText}",
        fontColor: "blue",                    
        fontSize: "12px",
        fontFamily: "Courier New, monospace",
        fontWeight: "bold",
        labelAlign: "cc",
        labelXOffset: "0",
        labelYOffset: "0",
        labelOutlineColor: "white",
        labelOutlineWidth: 3
});

var styleMapGrid = new OpenLayers.StyleMap({
    'pointRadius': 40
});

//---- ADD LAYERS ----//
var kmlLayer = null;

var layerUser = new OpenLayers.Layer.Vector("userVector", {styleMap: styleMap});
map.addLayer(layerUser);

var gridLayer = null;
var textLayer = null;


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

    //holds a list of stations for each map grid cell
    var stationCells = [];

    stationsWindow.document.write(stationFeatures.length+"<br>");

    //cell rows vertically
    for (var v=0; v<gridSideLength; v++)
    {
        //individual horizontal cells
        for (var h=0; h<gridSideLength; h++)
        {
            //make a list of all the features in this cell
            var featuresInCell = [];
            for (var s=0; s<stationFeatures.length; s++)
            {
                //determine if this station resides within the cell
                var point = new OpenLayers.LonLat(stationFeatures[s].geometry.x, stationFeatures[s].geometry.y);

                point.transform(toProjection, fromProjection);

                var cellOrigin = new OpenLayers.LonLat(gridOrigin.lon+gridWidth/gridSideLength*(h), gridOrigin.lat-gridHeight/gridSideLength*(v));

                if ((point.lon >= cellOrigin.lon)&&(point.lon <= cellOrigin.lon+gridWidth/gridSideLength)&&(point.lat <= cellOrigin.lat)&&(point.lat >= cellOrigin.lat-gridHeight/gridSideLength))
                {
                    //the station does reside within this cell
                    featuresInCell.push(stationFeatures[s]);
                }
            }
            //add all the features in this cell back to stationCells
            stationCells.push(featuresInCell);

            //write to the grid the number of stations in this cell
            if (gridLayer != null)
            {
                var point = new OpenLayers.Geometry.Point(gridOrigin.lon+gridWidth/gridSideLength*(h+0.15), gridOrigin.lat-gridHeight/gridSideLength*(v+0.15));
                point.transform(fromProjection,toProjection);
                //textLayer.addFeatures([new OpenLayers.Feature.Vector(point, {labelText: featuresInCell.length, fontColor: "red"})])
            }

        }
    }

    //write out the number of stations in each cell, space delimited
    for (var i=0; i<stationCells.length; i++)
    {
        stationsWindow.document.write(stationCells[i].length+" ");
    }

    stationsWindow.document.write("<br>");

    //write out this group of stations to the log
    for (var i=0; i<stationCells.length; i++)
    {
        for (var j=0; j<stationCells[i].length; j++)
        {
            var feature = stationCells[i][j];
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

            stationsWindow.document.write(callsign+" "+freq+" "+lat.toFixed(5)+" "+lon.toFixed(5)+" "+erp+" "+haat+"<br>");
        }

    }
    stationsWindow.focus();


}

function createGridLayer() {

    gridLayer = new OpenLayers.Layer.Vector("gridVector", {styleMap: styleMapGrid});
    map.addLayer(gridLayer);
    textLayer = new OpenLayers.Layer.Vector("textVector", {styleMap: styleMapText});
    map.addLayer(textLayer);

    //draw the grid to the gridLayer
        //draw the vertical lines
        for (var i=0; i<=gridSideLength; i++)
        {
            var start_point = new OpenLayers.Geometry.Point(gridOrigin.lon+gridWidth/gridSideLength*i, gridOrigin.lat);
            var end_point = new OpenLayers.Geometry.Point(gridOrigin.lon+gridWidth/gridSideLength*i, gridOrigin.lat-gridHeight);
            start_point.transform(fromProjection,toProjection);
            end_point.transform(fromProjection,toProjection);
            //gridLayer.addFeatures([start_point, end_point]);
            gridLayer.addFeatures([new OpenLayers.Feature.Vector(new OpenLayers.Geometry.LineString([start_point, end_point]))]);
        }
        //draw the horizontal lines
        for (var i=0; i<=gridSideLength; i++)
        {
            var start_point = new OpenLayers.Geometry.Point(gridOrigin.lon, gridOrigin.lat-gridHeight/gridSideLength*i);
            var end_point = new OpenLayers.Geometry.Point(gridOrigin.lon+gridWidth, gridOrigin.lat-gridHeight/gridSideLength*i);
            start_point.transform(fromProjection,toProjection);
            end_point.transform(fromProjection,toProjection);
            //gridLayer.addFeatures([start_point, end_point]);
            gridLayer.addFeatures([new OpenLayers.Feature.Vector(new OpenLayers.Geometry.LineString([start_point, end_point]))]);
        }
        //draw the cell numbers
        for (var i=0; i<gridSideLength; i++)
        {
            for (var j=0; j<gridSideLength; j++)
            {
                var point = new OpenLayers.Geometry.Point(gridOrigin.lon+gridWidth/gridSideLength*(j+0.5), gridOrigin.lat-gridHeight/gridSideLength*(i+0.5));
                point.transform(fromProjection,toProjection);
                textLayer.addFeatures([new OpenLayers.Feature.Vector(point, {labelText: j+10*i+1})])
            }
        }
}

function destroyGridLayer() {
    map.removeLayer(gridLayer);
    map.removeLayer(textLayer);
    gridLayer = null;
    textLayer = null;
}