const COORD = await import(`/maap/assets/js/gps/Coord.js`);
console.log('::COORD=<',COORD,'>');
const coord = new COORD.Coord();

Cesium.Ion.defaultAccessToken = `eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJjMWM0MTFlOC04OTljLTQyZDEtOGRkYS00M2EyMWY1MDRhY2UiLCJpZCI6MTAxNzk1LCJpYXQiOjE2NTgyNzk5ODF9.wS71k-QxR6CLoJ5l3VuJeb07sE3qOkkSgy2MbmuLFWg`;

document.addEventListener('DOMContentLoaded', async (evt) => {
  const posHistory = loadMapBySavedGpsAnchors();
  console.log('calcBestGPSFromHistory::posHistory=<',posHistory,'>');
  if(posHistory && posHistory.center) {
    createMapView(posHistory.center.lon,posHistory.center.lat);
  } else {
    createMapView(0.0,0.0);
  }
  if(posHistory && posHistory.anchor) {
    drawAnchorOnMap(posHistory.anchor);
  }
});

window.loadMapBySavedGpsAnchors = () => {
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
  const bestGpsAnchors = {};
  for(const anchorAddress in anchorsGpsHistory) {
    //console.log('loadMapBySavedGpsAnchors::anchorAddress=<',anchorAddress,'>');
    const gpsHistory = anchorsGpsHistory[anchorAddress];
    //console.log('loadMapBySavedGpsAnchors::gpsHistory=<',gpsHistory,'>');
    const gps = calcBestGPSFromHistory(gpsHistory);
    bestGps.push(gps);
    bestGpsAnchors[anchorAddress] = gps;
  }
  //console.log('loadMapBySavedGpsAnchors::bestGpsAnchors=<',bestGpsAnchors,'>');
  const gpsCenter = calcCenterOfGPS(bestGps);
  //console.log('loadMapBySavedGpsAnchors::gpsCenter=<',gpsCenter,'>');
  return {center:gpsCenter,anchor:bestGpsAnchors};
}


const drawAnchorOnMap = (anchorGps) => {
  //console.log('drawAnchorOnMap::anchorGps=<',anchorGps,'>');
  for(const anchorAddress in anchorGps) {
    console.log('drawAnchorOnMap::anchorAddress=<',anchorAddress,'>');
    const gps = anchorGps[anchorAddress];
    console.log('drawAnchorOnMap::gps=<',gps,'>');
    updateAnchorOnMap(anchorAddress,gps.lat,gps.lon,gps.geo);
    const xyz = coord.WGS2ECEF(gps.lat,gps.lon,gps.geo);
    console.log('onAnchorPosition::xyz=<',xyz,'>');
  }
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
  console.log('calcBestGPSFromHistory::remain3=<',remain3,'>');
  
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
  if(gpsPoints.length > 0) {
    bestGps.uwbid = gpsPoints[gpsPoints.length-1].uwbid;
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
  const options = {
    terrainProvider: Cesium.createWorldTerrain(),
    baseLayerPicker: true,
    timeline : false,
    animation : false,
    homeButton: false,
    vrButton: false,
    geocoder:false,
    sceneModePicker:false,
    navigationHelpButton:false,
  };
  gGPSMap = new Cesium.Viewer('view_map', options);
  const buildingTileset = gGPSMap.scene.primitives.add(Cesium.createOsmBuildings());
  gGPSMap.camera.flyTo({
    destination : Cesium.Cartesian3.fromDegrees(lat, lon, 80),
    orientation : {
      heading : Cesium.Math.toRadians(0.0),
      pitch : Cesium.Math.toRadians(-90.0),
    }
  });
}

const gGPSCurrentStyles = {};
let gGPSCurrentColorTableIndex = 0;
const gGPSColorTable = [
  Cesium.Color.RED,
  Cesium.Color.BROWN,
  Cesium.Color.CYAN,
  Cesium.Color.DARKRED,
];

window.updateMap = (lat,lon,address) => {
}

let gAnchorColorTableIndex = 0;
const gAnchorColor = {};
const updateAnchorOnMap = (address,lat,lon,geo) => {
  let color;
  if(!gAnchorColor[address]) {
    color = gGPSColorTable[gAnchorColorTableIndex];
    gAnchorColorTableIndex++;
    gAnchorColorTableIndex %= gGPSColorTable.length;
  }
  const positionAnchor = Cesium.Cartesian3.fromDegrees(lon,lat,geo);
  console.log('updateAnchorOnMap::positionAnchor=<',positionAnchor,'>');
  const billboardAnchor = {
    image :'./icons/anchor-solid.svg',
    width : 16,
    height : 16,
    color :color,
  };
  const entry = {
    name:address,
    description:address,
    position: positionAnchor,
    billboard : billboardAnchor
  }
  const point = gGPSMap.entities.add(entry);
  console.log('updateAnchorOnMap::point=<',point,'>');
}
