/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#import "CHIPUIViewUtils.h"

@implementation CHIPUIViewUtils

+ (UILabel *)addTitle:(NSString *)title toView:(UIView *)view
{
    UILabel * titleLabel = [UILabel new];
    titleLabel.text = title;
    titleLabel.textColor = UIColor.blackColor;
    titleLabel.textAlignment = NSTextAlignmentCenter;
    titleLabel.font = [UIFont systemFontOfSize:25 weight:UIFontWeightBold];
    [view addSubview:titleLabel];

    titleLabel.translatesAutoresizingMaskIntoConstraints = false;
    [titleLabel.centerXAnchor constraintEqualToAnchor:view.centerXAnchor].active = YES;
    [titleLabel.topAnchor constraintEqualToAnchor:view.topAnchor constant:110].active = YES;

    return titleLabel;
}

+ (UIStackView *)stackViewWithLabel:(UILabel *)label result:(UILabel *)result
{
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionEqualSpacing;

    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 3;

    label.textColor = UIColor.systemBlueColor;
    result.textColor = UIColor.systemBlueColor;
    label.font = [UIFont systemFontOfSize:17];
    result.font = [UIFont italicSystemFontOfSize:17];

    [stackView addArrangedSubview:label];
    [stackView addArrangedSubview:result];

    label.translatesAutoresizingMaskIntoConstraints = false;
    result.translatesAutoresizingMaskIntoConstraints = false;

    return stackView;
}

+ (UIView *)viewWithLabel:(UILabel *)label textField:(UITextField *)textField
{
    UIView * containingView = [UIView new];

    label.font = [UIFont systemFontOfSize:17];
    [containingView addSubview:label];

    label.translatesAutoresizingMaskIntoConstraints = false;
    [label.leadingAnchor constraintEqualToAnchor:containingView.leadingAnchor].active = true;
    [label.topAnchor constraintEqualToAnchor:containingView.topAnchor].active = true;
    [label.bottomAnchor constraintEqualToAnchor:containingView.bottomAnchor].active = YES;

    textField.font = [UIFont systemFontOfSize:17];
    textField.borderStyle = UITextBorderStyleRoundedRect;
    textField.textColor = UIColor.blackColor;
    [textField.layer setCornerRadius:14.0f];
    [containingView addSubview:textField];

    textField.translatesAutoresizingMaskIntoConstraints = false;
    [textField.trailingAnchor constraintEqualToAnchor:containingView.trailingAnchor].active = true;
    [textField.topAnchor constraintEqualToAnchor:containingView.topAnchor].active = true;
    [textField.bottomAnchor constraintEqualToAnchor:containingView.bottomAnchor].active = YES;
    [textField.leadingAnchor constraintEqualToAnchor:label.trailingAnchor constant:20].active = YES;
    return containingView;
}

+ (UIView *)viewWithLabel:(UILabel *)label toggle:(UISwitch *)toggle
{
    UIView * containingView = [UIView new];

    label.font = [UIFont systemFontOfSize:17];
    [containingView addSubview:label];

    label.translatesAutoresizingMaskIntoConstraints = false;
    [label.leadingAnchor constraintEqualToAnchor:containingView.leadingAnchor].active = true;
    [label.topAnchor constraintEqualToAnchor:containingView.topAnchor].active = true;
    [label.bottomAnchor constraintEqualToAnchor:containingView.bottomAnchor].active = YES;

    [containingView addSubview:toggle];

    toggle.translatesAutoresizingMaskIntoConstraints = false;
    [toggle.trailingAnchor constraintEqualToAnchor:containingView.trailingAnchor].active = true;
    [toggle.topAnchor constraintEqualToAnchor:containingView.topAnchor].active = true;
    [toggle.bottomAnchor constraintEqualToAnchor:containingView.bottomAnchor].active = YES;
    [toggle.leadingAnchor constraintGreaterThanOrEqualToAnchor:label.trailingAnchor constant:20].active = YES;

    return containingView;
}

+ (UIView *)viewWithUITextField:(UITextField *)textField button:(UIButton *)button
{
    UIView * containingView = [UIView new];

    textField.font = [UIFont systemFontOfSize:17];
    textField.backgroundColor = UIColor.whiteColor;
    textField.borderStyle = UITextBorderStyleRoundedRect;
    textField.textColor = UIColor.blackColor;
    [textField.layer setCornerRadius:14.0f];
    [containingView addSubview:textField];

    textField.translatesAutoresizingMaskIntoConstraints = false;
    [textField.leadingAnchor constraintEqualToAnchor:containingView.leadingAnchor].active = true;
    [textField.topAnchor constraintEqualToAnchor:containingView.topAnchor].active = true;
    [textField.bottomAnchor constraintEqualToAnchor:containingView.bottomAnchor].active = YES;

    button.titleLabel.font = [UIFont systemFontOfSize:17];
    button.titleLabel.textColor = [UIColor blackColor];
    button.layer.cornerRadius = 5;
    button.clipsToBounds = YES;
    button.backgroundColor = UIColor.systemBlueColor;
    [containingView addSubview:button];

    button.translatesAutoresizingMaskIntoConstraints = false;
    [button.trailingAnchor constraintEqualToAnchor:containingView.trailingAnchor].active = true;
    [button.topAnchor constraintEqualToAnchor:containingView.topAnchor].active = true;
    [button.bottomAnchor constraintEqualToAnchor:containingView.bottomAnchor].active = YES;
    [button.widthAnchor constraintEqualToConstant:60].active = YES;
    [button.leadingAnchor constraintEqualToAnchor:textField.trailingAnchor constant:15].active = YES;

    return containingView;
}

+ (UIStackView *)stackViewWithButtons:(NSArray<UIButton *> *)buttons
{
    UIStackView * stackViewButtons = [UIStackView new];
    stackViewButtons.axis = UILayoutConstraintAxisHorizontal;
    stackViewButtons.distribution = UIStackViewDistributionFillEqually;
    stackViewButtons.alignment = UIStackViewAlignmentTrailing;
    stackViewButtons.spacing = 10;

    for (int i = 0; i < buttons.count; i++) {
        UIButton * buttonForStack = [buttons objectAtIndex:i];
        buttonForStack.backgroundColor = UIColor.systemBlueColor;
        buttonForStack.titleLabel.font = [UIFont systemFontOfSize:17];
        buttonForStack.titleLabel.textColor = [UIColor whiteColor];
        buttonForStack.layer.cornerRadius = 5;
        buttonForStack.clipsToBounds = YES;
        buttonForStack.translatesAutoresizingMaskIntoConstraints = false;
        [buttonForStack.widthAnchor constraintEqualToConstant:70].active = YES;
        [stackViewButtons addArrangedSubview:buttonForStack];
    }
    return stackViewButtons;
}

+ (UIStackView *)stackViewWithLabel:(UILabel *)label buttons:(NSArray<UIButton *> *)buttons
{
    // Button stack view
    UIStackView * stackViewButtons = [UIStackView new];
    stackViewButtons.axis = UILayoutConstraintAxisHorizontal;
    stackViewButtons.distribution = UIStackViewDistributionEqualSpacing;
    stackViewButtons.alignment = UIStackViewAlignmentLeading;
    stackViewButtons.spacing = 10;

    label.font = [UIFont systemFontOfSize:17];
    [stackViewButtons addArrangedSubview:label];

    label.translatesAutoresizingMaskIntoConstraints = false;
    [label.centerYAnchor constraintEqualToAnchor:stackViewButtons.centerYAnchor].active = YES;

    stackViewButtons.translatesAutoresizingMaskIntoConstraints = false;
    for (int i = 0; i < buttons.count; i++) {
        UIButton * buttonForStack = [buttons objectAtIndex:i];
        buttonForStack.backgroundColor = UIColor.systemBlueColor;
        buttonForStack.titleLabel.font = [UIFont systemFontOfSize:17];
        buttonForStack.titleLabel.textColor = [UIColor whiteColor];
        buttonForStack.layer.cornerRadius = 5;
        buttonForStack.clipsToBounds = YES;
        buttonForStack.translatesAutoresizingMaskIntoConstraints = false;
        [buttonForStack.widthAnchor constraintGreaterThanOrEqualToConstant:60].active = YES;
        [stackViewButtons addArrangedSubview:buttonForStack];
    }

    return stackViewButtons;
}
@end
