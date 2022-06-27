document.addEventListener('AppScriptLoaded', async (evt) => {
  createMapView(0.0,0.0);
});

let gGPSMap = false;
const createMapView = (lat,lon) => {
  const layer =  new ol.layer.Tile({
    source: new ol.source.OSM()
  });
  const view = new ol.View({
    center: ol.proj.fromLonLat([lat, lon]),
    zoom: 20
  })  
  const mapOption = {
    target: 'view_map',
    layers: [
      layer
    ],
    view: view
  };
  gGPSMap = new ol.Map(mapOption);
}

let gGPSMapCenterOfRealGps = false;
const updateMap = (lat,lon) => {
  if(gGPSMapCenterOfRealGps === false) {
    gGPSMap.getView().setCenter(ol.proj.transform([lon, lat], 'EPSG:4326', 'EPSG:3857'));
    gGPSMapCenterOfRealGps = true;
  }
  const layerMarker = new ol.layer.Vector({
     source: new ol.source.Vector({
       features: [
         new ol.Feature({
             geometry: new ol.geom.Point(ol.proj.fromLonLat([lon, lat]))
         })
       ]
     })
   });
  gGPSMap.addLayer(layerMarker);  
}