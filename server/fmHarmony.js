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

var fromProjection = new OpenLayers.Projection("EPSG:4326"); // transform from WGS 1984
var toProjection = new OpenLayers.Projection("EPSG:900913"); // to Spherical Mercator Projection

var mapScale = new OpenLayers.Control.ScaleLine({
                minWidth: 75,
                maxWidth: 350
            });

// ------ CREATE THE MAP ------ //
var map = new OpenLayers.Map({
    div: "map",
    transitionEffect: null,
    zoomMethod: null,
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
        //new OpenLayers.Control.PanZoomBar(),
        mapScale,
        //new OpenLayers.Control.LayerSwitcher(),
        new OpenLayers.Control.KeyboardDefaults(),
        new OpenLayers.Control.Attribution()
    ]
});

var zoomTo = 13;

map.setCenter(new OpenLayers.LonLat(-69.064,44.210).transform(fromProjection,toProjection),zoomTo);