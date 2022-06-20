const MASS = await import(`./mass.js`);
export class Graviton {
  constructor() {
    if(Graviton.debug) {
      console.log('Graviton::constructor:MASS=<',MASS,'>');
    }
    if(Graviton.debug) {
      console.log('Graviton::constructor:this.mass_=<',this.mass_,'>');
    }
  }
  static debug = true;
}

document.addEventListener('AppScriptLoaded', async (evt) => {
  loadGravitonChannel_(evt);
});
const loadGravitonChannel_ = (evt) => {
  console.log('loadGravitonChannel_::evt=<',evt,'>');  
}
