#
#               Copyright 2007 Mentor Graphics Corporation
#                         All Rights Reserved.
#
# THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
# PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS SUBJECT
# TO LICENSE TERMS.
#

OBJS = \
$(OBJDIR)/ifxui_engine.o \
$(OBJDIR)/ifxui_rtl.o \
$(OBJDIR)/ifxui_uriparser.o \
$(OBJDIR)/LcCAggregate.o \
$(OBJDIR)/LcCAnimator.o \
$(OBJDIR)/LcCApplet.o \
$(OBJDIR)/LcCBase.o \
$(OBJDIR)/LcCBitmap.o \
$(OBJDIR)/LcCBitmapFrame.o \
$(OBJDIR)/LcCBitmappedFont.o \
$(OBJDIR)/LcCBitmappedText.o \
$(OBJDIR)/LcCCleanup.o \
$(OBJDIR)/LcCCustomBitmap.o \
$(OBJDIR)/LcCCustomBitmapFile.o \
$(OBJDIR)/LcCEnvironment.o \
$(OBJDIR)/LcCFont.o \
$(OBJDIR)/LcCKeyValueList.o \
$(OBJDIR)/LcCLight.o \
$(OBJDIR)/LcCMesh.o \
$(OBJDIR)/LcCMutex.o \
$(OBJDIR)/LcCNativeFont.o \
$(OBJDIR)/LcCNativeText.o \
$(OBJDIR)/LcCNdiBitmap.o \
$(OBJDIR)/LcCNdiBitmapFile.o \
$(OBJDIR)/LcCNdiFont.o \
$(OBJDIR)/LcCNdiGraphics.o \
$(OBJDIR)/LcCNdiNativeFont.o \
$(OBJDIR)/LcCNdiNativeText.o \
$(OBJDIR)/LcCNdiText.o \
$(OBJDIR)/LcCPlatformFileSearch.o \
$(OBJDIR)/LcCReadOnlyFile.o \
$(OBJDIR)/LcCRomFileSearch.o \
$(OBJDIR)/LcCShaHash.o \
$(OBJDIR)/LcCSpace.o \
$(OBJDIR)/LcCText.o \
$(OBJDIR)/LcCTokenReplacer.o \
$(OBJDIR)/LcCWidget.o \
$(OBJDIR)/LcCXml.o \
$(OBJDIR)/LcOglCBitmap.o \
$(OBJDIR)/LcOglCContext.o \
$(OBJDIR)/LcOglCCubeTexture.o \
$(OBJDIR)/LcOglCCustomBitmap.o \
$(OBJDIR)/LcOglCEffect.o \
$(OBJDIR)/LcOglCFont.o \
$(OBJDIR)/LcOglCGlobalState.o \
$(OBJDIR)/LcOglCGPUCaps.o \
$(OBJDIR)/LcOglCLight.o \
$(OBJDIR)/LcOglCMesh.o \
$(OBJDIR)/LcOglCNativeFont.o \
$(OBJDIR)/LcOglCNativeText.o \
$(OBJDIR)/LcOglCNdiBitmap.o \
$(OBJDIR)/LcOglCNdiFont.o \
$(OBJDIR)/LcOglCText.o \
$(OBJDIR)/LcOglCTexture.o \
$(OBJDIR)/LcTMessage.o \
$(OBJDIR)/LcTPixelRect.o \
$(OBJDIR)/LcTPlacement.o \
$(OBJDIR)/LcTPlaneRect.o \
$(OBJDIR)/LcTQuaternion.o \
$(OBJDIR)/LcTScalarQuad.o \
$(OBJDIR)/LcTScalarRect.o \
$(OBJDIR)/LcTString.o \
$(OBJDIR)/LcTTransform4.o \
$(OBJDIR)/LcTTransform.o \
$(OBJDIR)/LcTVector.o \
$(OBJDIR)/LcwCImage.o \
$(OBJDIR)/LcwCLabel.o \
$(OBJDIR)/LcCMarqueeRenderer.o \
$(OBJDIR)/LcwCLight.o \
$(OBJDIR)/LcwCMesh.o \
$(OBJDIR)/LcwCPlugin.o \
$(OBJDIR)/NdhsCAppUi.o \
$(OBJDIR)/NdhsCComponent.o \
$(OBJDIR)/NdhsCDragRegionElement.o \
$(OBJDIR)/NdhsCEffect.o \
$(OBJDIR)/NdhsCElement.o \
$(OBJDIR)/NdhsCElementGroup.o \
$(OBJDIR)/NdhsCEntryPointMapStack.o \
$(OBJDIR)/NdhsCExclusivity.o \
$(OBJDIR)/NdhsCExpression.o \
$(OBJDIR)/NdhsCExpressionField.o \
$(OBJDIR)/NdhsCExpressionFunction.o \
$(OBJDIR)/NdhsCField.o \
$(OBJDIR)/NdhsCFieldBool.o \
$(OBJDIR)/NdhsCFieldCache.o \
$(OBJDIR)/NdhsCFieldFloat.o \
$(OBJDIR)/NdhsCFieldInt.o \
$(OBJDIR)/NdhsCFieldTime.o \
$(OBJDIR)/NdhsCGraphicElement.o \
$(OBJDIR)/NdhsCKeyFrameList.o \
$(OBJDIR)/NdhsCLaundry.o \
$(OBJDIR)/NdhsCLightElement.o \
$(OBJDIR)/NdhsCManifest.o \
$(OBJDIR)/NdhsCManifestStack.o \
$(OBJDIR)/NdhsCMemoryImage.o \
$(OBJDIR)/NdhsCMenu.o \
$(OBJDIR)/NdhsCMenuComponent.o \
$(OBJDIR)/NdhsCMenuComponentTemplate.o \
$(OBJDIR)/NdhsCMenuCon.o \
$(OBJDIR)/NdhsCMenuItem.o \
$(OBJDIR)/NdhsCMenuWidgetFactory.o \
$(OBJDIR)/NdhsCPageManager.o \
$(OBJDIR)/NdhsCPageModel.o \
$(OBJDIR)/NdhsCPageTemplate.o \
$(OBJDIR)/NdhsCPage.o \
$(OBJDIR)/NdhsCPath.o \
$(OBJDIR)/NdhsCPlugin.o \
$(OBJDIR)/NdhsCPluginElement.o \
$(OBJDIR)/NdhsCPluginElementDirectView.o \
$(OBJDIR)/NdhsCPluginElementBufferedView.o \
$(OBJDIR)/NdhsCPluginElementOGLTextureView.o \
$(OBJDIR)/NdhsCPluginElementView.o \
$(OBJDIR)/NdhsCProjectValidator.o \
$(OBJDIR)/NdhsCScrollPosField.o \
$(OBJDIR)/NdhsCStateManager.o \
$(OBJDIR)/NdhsCTemplate.o \
$(OBJDIR)/NdhsCTextElement.o \
$(OBJDIR)/NdhsCTokenStack.o \
$(OBJDIR)/NdhsCTransitionAgent.o \
$(OBJDIR)/NdhsIntegrationInternals.o

ifdef ENABLE_OPENGL_20
OBJS += \
$(OBJDIR)/DefaultShaders-1.0.0_FileTable.o
endif

LIBRARY = $(BUILDDIR)/lib/libInflexionUIEngine.a
default: all
include $(BUILDDIR)/inflexionui/library.mk
