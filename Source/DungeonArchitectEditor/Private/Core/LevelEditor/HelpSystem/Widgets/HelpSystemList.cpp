//$ Copyright 2015-24, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/HelpSystem/Widgets/HelpSystemList.h"

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemStyle.h"

#include "Framework/SlateDelegates.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "HelpSystemList"

void SHelpSystemList::Construct(const FArguments& InArgs) {
    FText Title = FText::FromString("Test Title");
    FText Description = FText::FromString("Test Description");

    float ItemHeight = 80;

    struct Local {
        static FReply OpenURL(FString Link) {
            FPlatformProcess::LaunchURL(*Link, nullptr, nullptr);
            return FReply::Handled();
        }
    };

    static const FString UrlUserGuide = "https://docs.dungeonarchitect.dev/unreal";
    static const FString UrlVideoTutorials = "https://www.youtube.com/watch?v=ZLIYg7xJLks&list=PLGMINEyXduyk9h9fNPdnW7D4sZVxYampp";
    static const FString UrlReleaseNotes = "https://gist.github.com/coderespawn/590e8bbb4adc9782cab8e48ae2a64864";
    static const FString UrlDiscord = "https://discord.gg/x7e5cxH";
    static const FString UrlForumThread = "https://forums.unrealengine.com/showthread.php?67599-Dungeon-Architect";
    static const FString UrlRatePlugin = "https://www.unrealengine.com/marketplace/dungeon-architect";

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("DocumentationTitle", "Documentation"),
                LOCTEXT("DocumentationDesc", "Learn how to use Dungeon Architect"),
                "DungeonArchitect.HelpSystem.IconUserGuide",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlUserGuide)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("VideoTutorialsTitle", "Video Tutorials"),
                LOCTEXT("VideoTutorialsDesc", "Watch the video tutorials"),
                "DungeonArchitect.HelpSystem.IconVideoTutorials",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlVideoTutorials)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("ChangeLogTitle", "Release Notes"),
                LOCTEXT("ChangeLogDesc", "Find what\'s new in the latest release of the plugin"),
                "DungeonArchitect.HelpSystem.IconReleaseNotes",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlReleaseNotes)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("DiscordCommunityTitle", "Discord Community"),
                LOCTEXT("DiscordCommunityDesc",
                        "Join our Discord channel and chat with the developer and the community"),
                "DungeonArchitect.HelpSystem.IconDiscordChat",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlDiscord)
            ).ToSharedRef()
        ]
        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("ForumThreadTitle", "Forums Support Thread / Dev Log"),
                LOCTEXT("ForumThreadDesc",
                        "Follow the latest progress of Dungeon Architect and discuss anything regarding the plugin in the forum thread"),
                "DungeonArchitect.HelpSystem.IconForumThread",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlForumThread)
            ).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .MaxHeight(ItemHeight)
        [
            CreateEntry(
                LOCTEXT("RateTitle", "Rate us in the Marketplace"),
                LOCTEXT("RateDesc", "Let us know what you think of the plugin by rating in the marketplace"),
                "DungeonArchitect.HelpSystem.IconRatePlugin",
                FOnClicked::CreateStatic(&Local::OpenURL, UrlRatePlugin)
            ).ToSharedRef()
        ]
    ];
}


TSharedPtr<SWidget> SHelpSystemList::CreateEntry(const FText& Title, const FText& Description, const FName& IconId,
                                                 FOnClicked OnClicked) {
    //const FSlateBrush* SlateBrush = FAppStyle::Get().GetBrush("Tutorials.Browser.DefaultTutorialIcon");
    const FSlateBrush* SlateBrush = FDungeonArchitectHelpSystemStyle::Get().GetBrush(IconId);
    return
        SNew(SBox)
        .Padding(FMargin(0.0f, 2.0f))
        [
            SNew(SButton)
			.OnClicked(OnClicked)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Tutorials.Browser.Button"))
			.ForegroundColor(FSlateColor::UseForeground())
			.Content()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .VAlign(VAlign_Center)
                  .HAlign(HAlign_Center)
                  .Padding(8.0f)
                [
                    SNew(SOverlay)
                    + SOverlay::Slot()
                    [
                        SNew(SBox)
						.WidthOverride(64.0f)
						.HeightOverride(64.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
                        [
                            SNew(SImage)
                            .Image(SlateBrush)
                        ]
                    ]
                ]
                + SHorizontalBox::Slot()
                  .FillWidth(1.0f)
                  .VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
						.Text(Title)
						.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>(
                                            "Tutorials.Browser.SummaryHeader"))
						//.HighlightText(HighlightText)
						.HighlightColor(FAppStyle::Get().GetColor("Tutorials.Browser.HighlightTextColor"))
						.HighlightShape(FAppStyle::Get().GetBrush("TextBlock.HighlightShape"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        SNew(STextBlock)
						.AutoWrapText(true)
						.Text(Description)
						.TextStyle(
                                            &FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>(
                                                "Tutorials.Browser.SummaryText"))
						//.HighlightText(HighlightText)
						.HighlightColor(FAppStyle::Get().GetColor("Tutorials.Browser.HighlightTextColor"))
						.HighlightShape(FAppStyle::Get().GetBrush("TextBlock.HighlightShape"))
                    ]
                ]
            ]
        ];
}

#undef LOCTEXT_NAMESPACE

