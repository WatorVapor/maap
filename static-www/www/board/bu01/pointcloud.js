const model3d = {
  debug:false
};

window.addEventListener('DOMContentLoaded', () =>{
  loadPointCloud();
});

let gAppPointCloud = false;
const loadPointCloud = () => {
  gAppPointCloud = new PointCloud4Uwb('canvas-uwb-point-cloud');
}


class PointCloud4Uwb {
  constructor(canvas) {
    this.canvasid_ = canvas;
    this.loadPointCloud_();
    this.anchor_ = {};
  }
  loadPointCloud_() {
    const renderer = new THREE.WebGLRenderer({
      canvas: document.querySelector(`#${this.canvasid_}`)
    });
    const width = renderer.domElement.width;
    const height = renderer.domElement.height;
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(width, height);
    this.scene_ = new THREE.Scene();
    const axes = new THREE.AxesHelper(10);
    this.scene_.add(axes);

    const gridHelper_detail = new THREE.GridHelper( 50, 100 ,0x111111,0x222222);
    this.scene_.add( gridHelper_detail ); 

    const gridHelper = new THREE.GridHelper( 50, 10 );
    this.scene_.add( gridHelper ); 

    const camera = new THREE.PerspectiveCamera(45,width / height,0.001,1000);
    //const camera = new THREE.OrthographicCamera(width / - 2, width / 2, height / 2, height / - 2,1,100);
    camera.position.set(50, 50, 50);
    
    const controls = new THREE.OrbitControls(camera, renderer.domElement);      
    const light = new THREE.AmbientLight(0xffffff );
    light.intensity = 10; 
    light.position.set(-10, -10, -10);
    this.scene_.add(light);

    this.CenterGeometry_ = new THREE.SphereGeometry( 0.05, 256, 256 );
    this.CenterMaterial_ = new THREE.MeshBasicMaterial( {color: 0xff0000} );
    const sphere  = new THREE.Mesh( this.CenterGeometry_, this.CenterMaterial_ );
    this.scene_.add(sphere);
    
    this.pointGeometry_ = new THREE.BoxGeometry( 0.02, 0.02, 0.02 );
    this.pointMaterial_ = new THREE.MeshBasicMaterial( {color: 0xff0000} );

    const tick = () => {
      controls.update();
      renderer.render(this.scene_, camera);
      requestAnimationFrame(tick);
    }
    tick();
  }
  onDistanceData(anchor,distance){
    //console.log('onDistanceData::anchor=<',anchor,'>');
    if(!this.anchor_[anchor]) {
      this.anchor_[anchor] = {x:0,y:0.1,z:0};
    }
    //console.log('onDistanceData::distance=<',distance,'>');
    const geometry = new THREE.TorusGeometry( distance,0.005,128,128 );
    const circle = new THREE.Mesh( geometry, this.pointMaterial_ );
    //console.log('onDistanceData::circle=<',circle,'>');
    circle.rotation.set(Math.PI/2,0,0);
    const anPos = this.anchor_[anchor];
    circle.position.set(anPos.x,anPos.y,anPos.z);
    this.scene_.add(circle);
  }
}

const onClickClearCloudPoint = (elme) => {
  console.log('onClickClearCloudPoint::elme=<',elme,'>');
  //console.log('onClickClearCloudPoint::gAppPointCloud.scene_=<',gAppPointCloud.scene_,'>');
  gAppPointCloud = new PointCloud4Uwb('canvas-uwb-point-cloud');
}
