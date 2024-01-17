# Matter Device Development Prerequisites

If you plan to develop a Matter end product, this page lists the prerequisites and next steps to facilitate your production journey through Matter.

## Become a CSA Member

Your organization must be an associate member or better to get your product certified by a CSA-approved testing facility. As a member, your organization will receive membership perks:

- Official resources to assist you in developing Matter products.

- Authorization to contribute to the [Matter Github repository](https://github.com/project-chip/connectedhomeip).

- Once approved, CSA will reserve a unique Vendor ID (VID) chosen by your organization. This VID will be needed to provision your device.

  - Your unique VID will be added to the [CSA Distributed Compliance Ledger](https://webui.dcl.csa-iot.org/) (DCL).

- [Matter Certification tool access](https://csa-iot.org/certification/tools/certification-tool/)

  - Allows you to evaluate your product for certification before the official certification process.

Become a member at [CSA Membership](https://csa-iot.org/become-member/). You can see a list of the different memberships offered at CSA.

## Develop Your Matter Application

Creating a Matter Application is an exciting journey. To start your journey, you need to understand the [Matter Fundamentals - Silabs](../general/FUNDAMENTALS_INTRO.md). We also provide two development paths that you can choose from (OpenThread/WiFi)

Once you are ready to develop your Matter Application, you should review the [Matter over WiFi](../nav_5_wifi.rst) or [Matter over WiFi](../nav_4_thread.rst), which provides detailed background and instructions for Matter developers working in either the Thread or Wi-Fi models. The Developer's Guide contains a deeper dive into development.

Additionally, Silicon Labs recommends that you become familiar with the Matter Specification documents:

- Matter Core Specification

- Matter Device Library Specification

- Matter Application Cluster Specification

These specifications can be downloaded from the [CSA's Specifications Download Request](https://csa-iot.org/developer-resource/specifications-download-request/) website.

## CSA Certification

Once a device/product is ready for production, you must have your product certified by a CSA-approved testing facility to certify compliance with the Matter Standard. Review the [CSA Certification Process](https://csa-iot.org/certification/why-certify/) to ensure a smooth and prompt Matter product certification.

By becoming a member of the CSA, access to their [Matter Certification Tool](https://csa-iot.org/certification/tools/certification-tool/) is granted. The Matter Certification Tool facilitates applying to certify your Matter product, after which you can create your application and upload your images and required documents. For additional resources on the certification process, refer to the [Certifying your Matter Product, an Overview](https://community.silabs.com/s/article/Certifying-your-Matter-Product-an-Overview?language=en_US) KBA. This KBA provides information on:

- Overview of Matter Certification Steps and milestones

- Thread Certification

- Matter Test Harness Guide

- Important Matter CSA Resources and a Q&A

Once your device is approved, your organization will be issued an official Certification Declaration (CD). This CD is a cryptographic document issued by CSA that confirms that your device has been certified. The CD is included in the attestation process sent by the Commissionee during device attestation. To view a list of authorized test facilities, check out the [CSA Authorized Testing Providers](https://csa-iot.org/certification/testing-providers/) summary. This CD is required to be injected during the manufacturing process if you are using the Silicon Labs CPMS process.  This certification process can take some time to complete. You should ensure that this time is accounted for during your development lifecycle to keep your product on schedule.

### Helpful CSA Links

- [CSA Membership](https://csa-iot.org/become-member/)

- [CSA Specifications Download Request page](https://csa-iot.org/developer-resource/specifications-download-request/)

- [CSA Pre-Certification Tool](https://csa-iot.org/certification/tools/certification-tool/)

- [CSA Authorized Testing Providers](https://csa-iot.org/certification/testing-providers/)

- [CSA Distributed Compliance Ledger (DCL)](https://webui.dcl.csa-iot.org/)

- [CSA Test Distributed Compliance Ledger (DCL)](https://testnet.iotledger.io/)

- [PAA Providers](https://csa-iot.org/certification/paa/)

## Standards Development Organizations (SDO) Membership

Matter is an application layer that works on top of other proven network technologies. Your organization may also be required to be a member of an SDO and be able to pass the certification processes that these organizations require. Examples of these SDOs are Bluetooth, WiFi, Thread, and others. Be sure to take these organizational requirements into account when planning your products. Refer to the [Certifying your Matter Product, an Overview](https://community.silabs.com/s/article/Certifying-your-Matter-Product-an-Overview?language=en_US) KBA.

## Ready for Production

Silicon Labs is the only IoT embedded solution provider offering a Custom Part Manufacturing Service (CPMS) to device makers. This secure provisioning service allows IoT device makers to order customized hardware straight from the factory via the [CPMS web portal](https://cpms.silabs.com/login). CPMS removes the numerous complexities, time, and expense of custom provisioning Matter devices at scale. To provide this solution, Silicon Labs has partnered with Kudelski Security to provide scalable access to Device Attestation Certificates (DACs) for Matter Devices.

When moving to production, if you decide to provision your devices at scale using CPMS and would like to learn more about Kudelski, refer to their [Matter-compliant certificate service](https://www.kudelski-iot.com/services-and-systems/matter-paa-pai). To use the Silicon Labs CPMS solution, contact [Kudelski](https://www.kudelski-iot.com/services-and-systems/matter-paa-pai) to create an account. When creating a Kudelski account and using CPMS for production, you will need to specify Silicon Labs as a requestor and recipient of DACs for any PAIs that will be programmed in the Silicon Labs manufacturing facilities.
