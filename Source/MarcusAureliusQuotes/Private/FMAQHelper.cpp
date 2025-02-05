#include "FMAQHelper.h"
#include "Editor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "LevelEditor.h"
#include "SLevelViewport.h"

#include "HttpModule.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "SMAQuoteWidget.h"

#include "TimerManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"

#include "MarcusAureliusQuotesLog.h"

#if 0

// maybe make it so that the plugin only compiles on windows
// if we use this it's only for windows
#define _AMD64_
#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <winuser.h>
#endif
/*
 * NOTE:
 * binds:
 * http request
 * window to timer
 * ticker
*/

FMAQHelper::FMAQHelper() {
  this->SetDefaults();
}

void FMAQHelper::SetDefaults() {
  NumQuotes_ = 99;
  QuoteTick_ = 9.f;
  WindowLifetime_ = QuoteTick_ * 0.7f;
  DisplayProbability = 1/3.f;
  DefaultWindowPos_ = FVector2D::ZeroVector;
  InitVpSize_ = FVector2D(1000, 1000);
  bDefaultWindowPosSet_ = false;
  bQuoteFetched_ = false;
  bWindowWasEverCreated_ = false;
}

FMAQHelper::~FMAQHelper() { this->FreeResources(); }

void FMAQHelper::FreeResources() {
  if (GEditor) {
    GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
  }
  DestroyAllBrushes();
  KillWindow();
  QuotesReset();
  if (GEngine) {
    FWorldDelegates::OnWorldTickStart.Remove(MakeWindowTicker_);
    FTSTicker::GetCoreTicker().RemoveTicker(QuoteTicker_);
  }
}

void FMAQHelper::KillWindow() {
  if (auto SlateWindow = SlateWindowWP.Pin()) {
    SlateWindow->RequestDestroyWindow();
    SlateWindow.Reset();
    if (auto WindowContent = WindowContentWP.Pin()) {
      WindowContent.Reset();
    }
  }
}

bool FMAQHelper::CheckAllBrushes() {

  // NOTE: if just one texture is invalid, invalidate everything
  // because we destroy
  bool DoInvalidation = false;
  for (auto &Pair : CachedBrushes_) {
    if (!BrushCheck(Pair.Value)) {
      DoInvalidation = true;
      break;
    }
  }

  if (DoInvalidation) {
    for (auto &CachedTexture : CachedTextures_)
      CachedTexture = nullptr;
    CachedTextures_.Empty();
    for (auto &Pair : CachedBrushes_)
      Pair.Value.Reset();
  }
  return DoInvalidation;
}

void FMAQHelper::DestroyAllBrushes(){

  for (auto & Pair : CachedBrushes_) {
    DestroyBrush(Pair.Value);
  }
  for (auto& TextureObject : CachedTextures_)
  {
    TextureObject = nullptr;
  }
  CachedTextures_.Empty();
}
bool FMAQHelper::BrushCheck(TSharedPtr<FSlateBrush> &CachedAuthorImgBrush) {
  if (!CachedAuthorImgBrush.IsValid())
    return false;
  UTexture2D *CachedTexture =
      Cast<UTexture2D>(CachedAuthorImgBrush->GetResourceObject());

  if (!CachedTexture)
    return false;

  return !CachedTexture->HasAnyFlags(EObjectFlags::RF_BeginDestroyed);
}
void FMAQHelper::DestroyBrush(TSharedPtr<FSlateBrush>& CachedAuthorImgBrush){

  if (CachedAuthorImgBrush.IsValid()) {
    if (UTexture2D* CachedTexture = Cast<UTexture2D>(CachedAuthorImgBrush->GetResourceObject())) {
      if (!CachedTexture->HasAnyFlags(EObjectFlags::RF_BeginDestroyed)) {
        CachedTexture->RemoveFromRoot();
      }
      // CachedTexture = nullptr;
      CachedAuthorImgBrush->SetResourceObject(nullptr);
    }
    CachedAuthorImgBrush.Reset();
  }
}

void FMAQHelper::OnWorldTickStart(UWorld*, ELevelTick TickType, float DeltaTime)
{
  if (GEditor && !GEditor->PlayWorld)
  {
    if(!bWindowWasEverCreated_)
    {
      FLevelEditorModule &LevelEditorModule =
        FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
      InitQuoteWindow(LevelEditorModule.GetFirstLevelEditor());
    }
  }
}

void FMAQHelper::InitQuoteWindow(TSharedPtr<ILevelEditor> InLevelEditor) {

  if (!InLevelEditor.IsValid())
    return;

  TSharedPtr<SLevelViewport> ActiveLevelViewport =
      InLevelEditor->GetActiveViewportInterface();
  if (!ActiveLevelViewport.IsValid())
    return;

  TWeakPtr<SViewport> ViewportWidgetWP =
      ActiveLevelViewport->GetViewportWidget();
  if (auto ViewportWidget = ViewportWidgetWP.Pin()) {
    const FGeometry &VPGeo = ViewportWidget->GetTickSpaceGeometry();
    const FVector2D GeoPosition = VPGeo.GetAbsolutePosition();
    const FVector2D GeoSize = VPGeo.GetAbsoluteSize();
    const FVector2D Padd(20, 20);
    float x = GeoPosition.X + GeoSize.X - Padd.X;
    float y = GeoPosition.Y + (GeoSize.Y / 2);
    FVector2D Result(x, y);
    FString ResultStr = Result.ToString();
#if 0
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Viewport pos = %s"),
           *ResultStr);
#endif
    DefaultWindowPos_ = Result;
    InitVpSize_ = GeoSize;
    bDefaultWindowPosSet_ = true;
    LoadAllAvatars();
    CreateSlateWindow();
  }
}

void FMAQHelper::CreateSlateWindow() {

  const float AppScale = FSlateApplication::Get().GetApplicationScale();
  const float DpiScaleFactor =
      FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(DefaultWindowPos_.X,
                                                         DefaultWindowPos_.Y);
#if 0
  UE_LOG(LogMarcusAureliusQuotes, Warning,
         TEXT("app scale = %f, dpi scale = %f"), AppScale, DpiScaleFactor);
#endif

  // NOTE:
  // - window size ratio is 3/4
  // - based on tests we want the X value to occupy about 1/4 of the VP size X
  const float InitWinSizeX = FMath::Max(200.f, float(InitVpSize_.X) * 0.24f * AppScale);
  const float InitWinSizeY = InitWinSizeX / 0.75f;
  const FVector2D DefaultScreenSize = FVector2D(InitWinSizeX, InitWinSizeY);
  const float DefaultAuthorImgRes = 64.f * AppScale;

  TSharedPtr<SMAQuoteWidget> WindowContent =
      SNew(SMAQuoteWidget)
      .DefaultWScreenSize(DefaultScreenSize)
      .DefaultAuthorImgRes(DefaultAuthorImgRes);

  WindowContentWP = WindowContent;

  TSharedPtr<SWindow> SlateWindow =
      SNew(SWindow)
          .Title(FText::FromString(TEXT("Quote")))
          .ClientSize(DefaultScreenSize)
		  .ScreenPosition({0., 0.})
          .SupportsMaximize(false)
          .SupportsMinimize(false)

          .IsTopmostWindow(false)
          .FocusWhenFirstShown(false)
          .ActivationPolicy(EWindowActivationPolicy::Never) /* does not work */
          .IsPopupWindow(false)
          .ShouldPreserveAspectRatio(false)[WindowContent.ToSharedRef()];
  SlateWindowWP = SlateWindow;
  // NOTE: you have to force it, unreal will place it in the center 
  // despite the position set in the construct args
  SlateWindow->MoveWindowTo(DefaultWindowPos_ - FVector2D(DefaultScreenSize.X, 0.));

  TSharedPtr<SWindow> ParentWindow =
      FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame")
          .GetParentWindow();
  if (ParentWindow.IsValid()) {
    // NOTE: when the window is attached to the application there is no need to
    // bring it to front
    FSlateApplication::Get().AddWindowAsNativeChild(
        SlateWindow.ToSharedRef(), ParentWindow.ToSharedRef(), false);
  } else {
    FSlateApplication::Get().AddWindow(SlateWindow.ToSharedRef(), false);
  }

  bWindowWasEverCreated_ = true;
}

void FMAQHelper::UpdateWindowSize() {

  if (auto SlateWindow = SlateWindowWP.Pin()) {

    const float DpiScaleFactor = SlateWindow->GetDPIScaleFactor();
    FVector2D WPos = SlateWindow->GetPositionInScreen();
    SlateWindow->SlatePrepass(
        FSlateApplicationBase::Get().GetApplicationScale() * DpiScaleFactor);
    const FVector2D WNewSize = SlateWindow->GetDesiredSize();
    FVector2D WRightBottomCorner = WPos + WNewSize;
    bool MoveIt = false;
    if(WRightBottomCorner.X > DefaultWindowPos_.X)
    {
      const float Offset = WRightBottomCorner.X - DefaultWindowPos_.X;
      WPos.X -= Offset;
      MoveIt = true;
    }

    SlateWindow->ReshapeWindow(WPos, WNewSize);
    if(MoveIt)
    {
      SlateWindow->SetCachedScreenPosition(WPos);
    }
    SlateWindow->SetCachedSize(WNewSize);

  }
}

void FMAQHelper::UpdateWindowQuote(const FString &_Quote,
                                   const FString &_Author) {
  auto SlateWindow = SlateWindowWP.Pin();
  auto WindowContent = WindowContentWP.Pin();
  if (SlateWindow.IsValid() && WindowContent.IsValid()) {
    WindowContent->SetQuote(FText::FromString(_Quote),
                            FText::FromString(_Author));
  }
}

bool FMAQHelper::CanDisplayQuote() {

  auto SlateWindow = SlateWindowWP.Pin();
  return SlateWindow.IsValid() && SlateWindow->GetVisibility().IsVisible();
}

// TODO: call before first window creation
void FMAQHelper::LoadAllAvatars() {

    TArray<TPair<FString, FName>> Avatars =
    { {"socrates64.png", "Any"},
      {"epictetus64.png", "Epictetus"},
      {"seneca64.png", "Seneca"},
      {"marcus-aurelius64.png", "Marcus Aurelius"},
    };

  const FString BaseDir = IPluginManager::Get()
                              .FindPlugin(TEXT("MarcusAureliusQuotes"))
                              ->GetBaseDir();

  const FString AvatarDir = FPaths::ConvertRelativePathToFull(
      FPaths::Combine(BaseDir, TEXT("Resources"), TEXT("stoics")));

  for (const auto &Pair : Avatars) {

    const FString AuthorImagePath = FPaths::Combine(AvatarDir, Pair.Key);

    // NOTE: this function is supposed to be called as initialization step
    // if key exist, take the brush and destroy it
    TSharedPtr<FSlateBrush> &CachedBrush =
        CachedBrushes_.Add(Pair.Value, {});
    TextureToBrush(AuthorImagePath, CachedBrush);
  }
}

void FMAQHelper::TextureToBrush(const FString &_Path, TSharedPtr<FSlateBrush> & CachedAuthorImgBrush) {

  // cached
  if (CachedAuthorImgBrush.IsValid())
    return;

  TArray<uint8> RawFileData;
  if (!FFileHelper::LoadFileToArray(RawFileData, *_Path)) {
    UE_LOG(LogMarcusAureliusQuotes, Error,
           TEXT("Failed to load image file: %s"), *_Path);
    return;
  }

  IImageWrapperModule &ImageWrapperModule =
      FModuleManager::LoadModuleChecked<IImageWrapperModule>(
          FName("ImageWrapper"));

  TSharedPtr<IImageWrapper> ImageWrapper =
      ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

  if (!ImageWrapper.IsValid() ||
      !ImageWrapper->SetCompressed(RawFileData.GetData(),
                                   sizeof(uint8) * RawFileData.Num())) {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Failed to decode image: %s"),
           *_Path);
    return;
  }

  TArray<uint8> RawData;
  if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, RawData)) {

    // NOTE: a naked transient texture will get killed on save
    UTexture2D *AuthorTexture = UTexture2D::CreateTransient(
        ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);

    AuthorTexture->AddToRoot();
    // CachedTexture = AuthorTexture ;
    CachedTextures_.Emplace(AuthorTexture);
    

    if (AuthorTexture) {

      void *TextureData =
          AuthorTexture->GetPlatformData()->Mips[0].BulkData.Lock(
              LOCK_READ_WRITE);

      FMemory::Memcpy(TextureData, RawData.GetData(),
                      sizeof(uint8) * RawData.Num());

      AuthorTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

      AuthorTexture->UpdateResource();
      CachedAuthorImgBrush = MakeShareable(new FSlateBrush());
      CachedAuthorImgBrush->SetResourceObject(AuthorTexture);
      CachedAuthorImgBrush->ImageSize =
          FVector2D(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
      CachedAuthorImgBrush->DrawAs = ESlateBrushDrawType::Image;
      CachedAuthorImgBrush->Tiling = ESlateBrushTileType::NoTile;
    }

  } else {

    UE_LOG(LogMarcusAureliusQuotes, Error,
           TEXT("Failed to get raw image data: %s"), *_Path);
    return;
  }
}

void FMAQHelper::DisplayQuote() {

  if (bQuoteFetched_ && Quotes.Num() && CanDisplayQuote()) {

    auto SlateWindow = SlateWindowWP.Pin();
    auto WindowContent = WindowContentWP.Pin();
    FMAQuote &&Quote = Quotes.Pop(true);

    FString AuthorPretty =
        FString::Format(TEXT("{0} {1}"), {TEXT("──"), *Quote.author});

    if (CheckAllBrushes()) {

      // NOTE: invalid now, reload
      UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("cache is invalid, building again"));
      LoadAllAvatars();
    }

    auto CachedAuthorBrushPtr = CachedBrushes_.Find(FName(Quote.author));

    // NOTE: fallback to some image
    if (!CachedAuthorBrushPtr)
      CachedAuthorBrushPtr = CachedBrushes_.Find(FName("Any"));

    if (CachedAuthorBrushPtr) {
      if ((*CachedAuthorBrushPtr).IsValid()) {
        WindowContent->SetAuthorImg((*CachedAuthorBrushPtr).Get());
      } else {

        UE_LOG(LogMarcusAureliusQuotes, Error,
               TEXT("No valid brush for author %s"), *Quote.author);
      }
    } else {
      UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("No avatar found"));
    }

    UpdateWindowQuote(Quote.quote, AuthorPretty);
    UpdateWindowSize();

    if (SlateWindow.IsValid()) {

      SlateWindow->ShowWindow();

#if 0
      HWND w = static_cast<HWND>(
          SlateWindow->GetNativeWindow()->GetOSWindowHandle());
      HWND HWndInsertAfter = HWND_TOP;
      uint32 Flags =
          SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE;

      ::SetWindowPos(w, HWndInsertAfter, 0, 0, 0, 0, Flags);
#endif

      // NOTE: if window is top-most this will
      // happen on ShowWindow()
      // SlateWindow->BringToFront(); 

      GEditor->GetTimerManager()->ClearTimer(AutoHideTimerHandle);
      auto &SlateWindowWeak = this->SlateWindowWP;
      auto l_HideWindow = [SlateWindowWeak]() {
        if (auto p = SlateWindowWeak.Pin()) {
          p->HideWindow();
        }
      };
      GEditor->GetTimerManager()->SetTimer(
          AutoHideTimerHandle, FTimerDelegate::CreateLambda(l_HideWindow),
          WindowLifetime_, false);
    }

    if (!Quotes.Num()) {
      QuotesReset();
    }
  } else {
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("No quotes to display"));
  }
}

void FMAQHelper::InitQuoteTickers() {

  MakeWindowTicker_ = FWorldDelegates::OnWorldTickStart.AddSP(
      AsShared(), &FMAQHelper::OnWorldTickStart);

  QuoteTicker_ = FTSTicker::GetCoreTicker().AddTicker(
      FTickerDelegate::CreateSP(AsShared(), &FMAQHelper::Tick), QuoteTick_);
}

bool FMAQHelper::DoDisplay()
{
  // NOTE: this is equivalent to bernouilli's apparently
  return FMath::FRand() <= FMath::Clamp(DisplayProbability, 0.0f, 1.0f);
}
bool FMAQHelper::Tick(float DeltaTime) {


  if(!bWindowWasEverCreated_)
  {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Failed to initialize window"));
    return true;
  }

  auto SlateWindow = SlateWindowWP.Pin();

  if(!SlateWindow.IsValid())
  {
    // Window has been destroyed, we re-create
    UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("WINDOW RECREATE"));
    CreateSlateWindow();
  }

  if (GEngine && GEditor) {

    if (DoDisplay()) {
      if (CanDisplayQuote()) {
        if (!bQuoteFetched_ && !FetchQuotes()) {
          UE_LOG(LogMarcusAureliusQuotes, Warning,
                 TEXT("Failed attempt to fetch quotes"));
          return true;
        }
        DisplayQuote();
      }
    }
  }
  return true;
}

void FMAQHelper::PrintAllQuotes(int Num, bool bFromBottom) {

  if (bQuoteFetched_) {
    int len = FMath::Min(Quotes.Num(), Num);
    for (size_t i = 0; i < len; i++) {

      FMAQuote Quote;
      if (bFromBottom) {
        Quote = Quotes[Quotes.Num() - i - 1];
      } else {
        Quote = Quotes[i];
      }
      const FString &currentQuote = Quote.quote;
      const FString &currentAuthor = Quote.author;

      UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("%s -- %s"), *currentQuote, *currentAuthor);
    }
  }
}
void FMAQHelper::QuotesReset() {
  Quotes.Empty();
  bQuoteFetched_ = false;
}

void FMAQHelper::SetRequestTimeout(const float _HowLong) {
  FHttpModule::Get().SetHttpTimeout(_HowLong);
}

bool FMAQHelper::FetchQuotes() {
  // int NumberOfQuotes = NumQuotes_;
  FString Url = TEXT("https://stoic-quotes.com/api/quotes?num=");
  Url += FString::FromInt(NumQuotes_);

  UE_LOG(LogMarcusAureliusQuotes, Warning, TEXT("Fetching quotes.. ."));
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
      FHttpModule::Get().CreateRequest();
  Request->SetVerb("GET");
  Request->SetURL(Url);

  Request->OnProcessRequestComplete().BindThreadSafeSP(this,
                                             &FMAQHelper::OnResponseReceived);

  return Request->ProcessRequest();
}

void FMAQHelper::OnResponseReceived(FHttpRequestPtr Request,
                                    FHttpResponsePtr Response,
                                    bool bWasSuccessful) {

  QuotesReset();

  auto OnError = [](const FString &InError) {
    UE_LOG(LogMarcusAureliusQuotes, Error, TEXT("Error: %s"), *InError);
  };

  auto OnSuccess = [this](const TArray<TSharedPtr<FJsonValue>> &JsonArray) {
    Quotes.Reserve(NumQuotes_);
    for (const auto &JsonVal : JsonArray) {
      auto &CurrentDict = JsonVal->AsObject();

      FString QuoteText;
      FString QuoteAuthor;
      FMAQuote QuoteObject;

      if (CurrentDict->TryGetStringField("text", QuoteText) &&
          CurrentDict->TryGetStringField("author", QuoteAuthor)) {
        QuoteObject.quote = QuoteText;
        QuoteObject.author = QuoteAuthor;
        Quotes.Add(QuoteObject);
      }
    }
    bQuoteFetched_ = bool(Quotes.Num());
  };

  if (!Response.IsValid() || !bWasSuccessful) {
    OnError(TEXT("Request failed"));
    return;
  }

  if (Response->GetResponseCode() < 200 || Response->GetResponseCode() > 299) {
    OnError(
        FString::Printf(TEXT("HTTP Error: %d"), Response->GetResponseCode()));
    return;
  }
  TArray<TSharedPtr<FJsonValue>> JsonArray;
  TSharedRef<TJsonReader<>> Reader =
      TJsonReaderFactory<>::Create(Response->GetContentAsString());

  if (FJsonSerializer::Deserialize(Reader, JsonArray)) {
    OnSuccess(JsonArray);
  } else {
    OnError(TEXT("Failed to parse JSON response"));
  }
}
