document.addEventListener('AppScriptLoaded', async (evt) => {
  const posHistory = loadMapBySavedGpsAnchors();
  console.log('calcBestGPSFromHistory::posHistory=<',posHistory,'>');
  if(posHistory) {
    createMapView(posHistory.lon,posHistory.lat);
  } else {
    createMapView(0.0,0.0);
  }
});

const loadMapBySavedGpsAnchors = () => {
  const anchorsGpsHistory = {};
  const keys = Object.keys(localStorage);
  //console.log('loadMapBySavedGpsAnchors::keys=<',keys,'>');
  for(const itemKey of keys) {
    if(itemKey.startsWith(constAnchorGpsHistoryPrefix)) {
      //console.log('loadMapBySavedGpsAnchors::itemKey=<',itemKey,'>');
      const addressAnchor = itemKey.replace(constAnchorGpsHistoryPrefix + '/','');
      //console.log('loadMapBySavedGpsAnchors::addressAnchor=<',addressAnchor,'>');
      const historyGpsStr = localStorage.getItem(itemKey);
      //console.log('loadMapBySavedGpsAnchors::historyGpsStr=<',historyGpsStr,'>');
      if(historyGpsStr) {
        const histroryGps = JSON.parse(historyGpsStr);
        if(histroryGps) {
          //console.log('loadMapBySavedGpsAnchors::histroryGps=<',histroryGps,'>');
          anchorsGpsHistory[addressAnchor] = histroryGps;
        }
      }
    }
  }
  //console.log('loadMapBySavedGpsAnchors::anchorsGpsHistory=<',anchorsGpsHistory,'>');
  const bestGps = [];
  for(const anchorAddress in anchorsGpsHistory) {
    //console.log('loadMapBySavedGpsAnchors::anchorAddress=<',anchorAddress,'>');
    const gpsHistory = anchorsGpsHistory[anchorAddress];
    //console.log('loadMapBySavedGpsAnchors::gpsHistory=<',gpsHistory,'>');
    const gps = calcBestGPSFromHistory(gpsHistory);
    bestGps.push(gps);
  }
  const gpsCenter = calcCenterOfGPS(bestGps);
  //console.log('loadMapBySavedGpsAnchors::gpsCenter=<',gpsCenter,'>');
  return gpsCenter;
}

const iConstFilterOutOnce = 6;
const iConstFilterOutOnce2 = iConstFilterOutOnce/2;
const calcBestGPSFromHistory = (gpsPoints)=> {
  //console.log('calcBestGPSFromHistory::gpsPoints=<',gpsPoints,'>');
  const lonSorted = gpsPoints.sort((a,b)=> { return a.lon - b.lon });
  //console.log('calcBestGPSFromHistory::lonSorted=<',lonSorted,'>');
  const cutStart = (lonSorted.length/iConstFilterOutOnce);
  const cutLength = lonSorted.length - (lonSorted.length/iConstFilterOutOnce2);
  const remain = lonSorted.slice(cutStart,cutLength);
  //console.log('calcBestGPSFromHistory::remain=<',remain,'>');

  const latSorted = remain.sort((a,b)=> { return a.lat - b.lat });
  const cutStart2 = (latSorted.length/iConstFilterOutOnce);
  const cutLength2 = latSorted.length - (latSorted.length/iConstFilterOutOnce2);
  const remain2 = latSorted.slice(cutStart2,cutLength2);
  //console.log('calcBestGPSFromHistory::remain2=<',remain2,'>');

  const geoSorted = remain2.sort((a,b)=> { return a.geo - b.geo });
  const cutStart3 = (geoSorted.length/iConstFilterOutOnce);
  const cutLength3 = geoSorted.length - (geoSorted.length/iConstFilterOutOnce2);
  const remain3 = geoSorted.slice(cutStart3,cutLength3);
  //console.log('calcBestGPSFromHistory::remain3=<',remain3,'>');
  
  const bestGps = {
    lon:0.0,
    lat:0.0,
    geo:0.0,
  };
  for(const point of remain3) {
    //console.log('calcBestGPSFromHistory::point=<',point,'>');
    bestGps.lon += point.lon;
    bestGps.lat += point.lat;
    bestGps.geo += point.geo;
  }
  //console.log('calcBestGPSFromHistory::bestGps=<',bestGps,'>');
  if(remain3.length > 0) {
    bestGps.lat /= remain3.length;
    bestGps.lon /= remain3.length;
    bestGps.geo /= remain3.length;
  }
  //console.log('calcBestGPSFromHistory::bestGps=<',bestGps,'>');
  return bestGps;
}

const calcCenterOfGPS = (gpsPoints)=> {
  const centerGps = {
    lon:0.0,
    lat:0.0,
    geo:0.0,
  };
  for(const point of gpsPoints) {
    centerGps.lon += point.lon;
    centerGps.lat += point.lat;
    centerGps.geo += point.geo;
  }
  if(gpsPoints.length > 0) {
    centerGps.lat /= gpsPoints.length;
    centerGps.lon /= gpsPoints.length;
    centerGps.geo /= gpsPoints.length;
  }
  console.log('calcCenterOfGPS::centerGps=<',centerGps,'>');
  return centerGps;
}


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