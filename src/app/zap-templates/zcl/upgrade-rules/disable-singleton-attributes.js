/**
 * Upgrade rule to notify users that no attributes in matter have singleton enabled.
 * @param {*} api
 * @param {*} context
 * @returns A string mentioning the singleton changes to cluster attributes made on a .zap file
 */
async function postLoad(api, context)
{
  let resMsg = "";
  const epts = await api.endpoints(context);
  for (const ept of epts) {
    let clusters = await api.clusters(context, ept);
    for (const cluster of clusters) {
      let attributes = await api.attributes(
          context,
          ept,
          cluster,
      );
      for (const attribute of attributes) {
        // None of the matter attributes are singleton
        if (attribute.isSingleton) {
          let params = [
            {
              key : context.updateKey.attributeSingleton,
              value : false,
            },
          ];
          await api.updateAttribute(
              context,
              ept,
              cluster,
              attribute,
              params,
          );
          resMsg += `${attribute.name} attribute of ${cluster.name} cluster on endpoint ${
              ept.endpointIdentifier} is no longer a singleton attribute.\n`;
        }
      }
    }
  }
  // Status can be 'nothing', 'automatic', 'user_verification', 'impossible'.
  if (resMsg) {
    console.log("ZAP file upgraded with the following changes:\n" + resMsg);
    return { message : resMsg, status : "automatic" };
  }
}

exports.postLoad = postLoad;
