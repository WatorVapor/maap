const onUISettingInfo = (info) => {
  APP.vm.wifi = info.wifi;
  APP.vm.mqtt = info.mqtt;
}
document.addEventListener('DOMContentLoaded',()=>{
  setTimeout(()=>{
    createVueApps();
  },1000);
})
const APP = {};
const setting_anchor_data = {
  wifi:{
    password: '',
    ssid: '',
  },
  mqtt:{
    address: '',
    jwt: '',
    url: '',
  }
}
const createVueApps = ()=> {
  const setting_option = {
    data() {
      return setting_anchor_data;
    },
    methods: {
      onClickChangeSetting(evt) {
        //console.log('onClickChangeSetting::evt=<',evt,'>');
        //console.log('onClickChangeSetting::this.wifi=<',this.wifi,'>');
        //console.log('onClickChangeSetting::this.mqtt=<',this.mqtt,'>');
        const uri = URI.parse(this.mqtt.url);
        const jwt = URI.parse(this.mqtt.jwt);
        //console.log('onClickChangeSetting::uri=<',uri,'>');
        //console.log('onClickChangeSetting::jwt=<',jwt,'>');
        const setting = {
          setting:{
            wifi:this.wifi,
            mqtt:this.mqtt,
            mqtt_:{
              url:{
                host:uri.host,
                port:uri.port,
                path:uri.resourceName,
              },
              jwt:{
                host:jwt.host,
                port:jwt.port,
                path:jwt.resourceName,
              }
            }
          }
        }
        writeJsonCmd(setting);
      },
    }
  };
  const settingApp = Vue.createApp(setting_option);
  APP.vm = settingApp.mount('#vue-ui-info-of-achor');  
}