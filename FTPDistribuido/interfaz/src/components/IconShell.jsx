import * as React from "react"
import styled from "styled-components"
import Octicon from "@primer/octicons-react"
import { boolToVisibility } from "./utils"

export const IconShell = ({ icon, size, visible = true, ...props }) => {
  return (
    <IconShellLayout {...{ ...props, visible }}>
      <Octicon icon={icon} size={size} />
    </IconShellLayout>
  )
}

const IconShellLayout = styled.div`
  visibility: ${({ visible }) => boolToVisibility(visible)};
`
