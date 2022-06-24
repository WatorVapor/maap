const onUISettingInfo = (info) => {
  if(info.wifi) {
    APP.vm.wifi = info.wifi;
  }
  if(info.mqtt) {
    APP.vm.mqtt = info.mqtt;
  }
  if(info.topic) {
    APP.vm.topic = info.topic;
  }
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
  },
  topic:{
    out:[],
  },
  mansions:[],
  mansionSelected:'',
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
      onClickAddMansion(evt) {
        console.log('onClickAddMansion::evt=<',evt,'>');
        console.log('onClickAddMansion::this.mansionSelected=<',this.mansionSelected,'>');
        if(!this.topic) {
          this.topic = {};
        }
        if(!this.topic.out) {
          this.topic.out = [];
        }
        this.topic.out.push(this.mansionSelected);
        this.topic.out = [...new Set(this.topic.out)];
        console.log('onClickAddMansion::this.topic.out=<',this.topic.out,'>');
        const setting = {
          setting:{
            topic:{
              out:this.topic.out
            }
          }
        }
        writeJsonCmd(setting);
      },
    }
  };
  const settingApp = Vue.createApp(setting_option);
  APP.vm = settingApp.mount('#vue-ui-info-of-anchor');  
  const starMansionListStr = localStorage.getItem(constMansionList);
  if(starMansionListStr) {
    const starMansionList = JSON.parse(starMansionListStr);    
    console.log('loadStarryDashboardApp_::starMansionList=<',starMansionList,'>');
    APP.vm.mansions = starMansionList;
  }

}