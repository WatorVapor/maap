const componentTag = 'w-navbar';
const componentURL = `${appPrefix}/assets/component/navbar.html`;
const componentData = {
  root:appPrefix,
  accout:{ 
    name:''
  }
};
const { default:createMaapVueApp } = await import(`${appPrefix}/assets/component/component.js`);
const createVueApp = async ()=> {
  return await createMaapVueApp(componentTag,componentURL,componentData);
}
export default createVueApp;

